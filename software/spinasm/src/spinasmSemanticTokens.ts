import * as vscode from 'vscode';

/**
 * @interface SpinASMSymbol
 * @brief Represents a symbol defined in SpinASM code (equ, mem, or label)
 */
interface SpinASMSymbol {
  name: string;
  type: 'register' | 'memory' | 'label' | 'constant';
  value?: string;
  line: number;
}

/**
 * @class SpinASMSemanticTokensProvider
 * @brief Provides semantic highlighting for user-defined SpinASM symbols
 *
 * This provider parses SpinASM files to identify:
 * - EQU declarations (register aliases and constants)
 * - MEM declarations (delay line names)
 * - Labels (jump targets)
 *
 * And highlights all uses of these symbols throughout the document.
 */
export class SpinASMSemanticTokensProvider implements vscode.DocumentSemanticTokensProvider {

  /**
   * @brief Legend defining token types and modifiers
   */
  static readonly legend = new vscode.SemanticTokensLegend(
    [
      'variable',      // 0: User-defined register aliases (equ)
      'property',      // 1: Memory buffer names (mem)
      'function',      // 2: Label definitions
      'parameter'      // 3: Constants
    ],
    [
      'declaration',   // 0: Symbol definition site
      'readonly'       // 1: Constant values
    ]
  );

  /**
   * @brief Provide semantic tokens for the entire document
   */
  provideDocumentSemanticTokens(
    document: vscode.TextDocument,
    token: vscode.CancellationToken
  ): vscode.ProviderResult<vscode.SemanticTokens> {

    const tokensBuilder = new vscode.SemanticTokensBuilder(
      SpinASMSemanticTokensProvider.legend
    );

    // Parse the document to find all symbol definitions (keyed by lowercase name)
    const symbols = this.parseSymbols(document);

    if (symbols.size === 0) {
      return tokensBuilder.build();
    }

    // Build a single combined regex for all symbols (case-insensitive)
    const escapedNames = Array.from(symbols.keys()).map(name => this.escapeRegex(name));
    const combinedRegex = new RegExp(`\\b(${escapedNames.join('|')})\\b`, 'gi');

    // Scan each line once with the combined regex
    for (let lineIndex = 0; lineIndex < document.lineCount; lineIndex++) {
      if (token.isCancellationRequested) {
        return;
      }

      const lineText = document.lineAt(lineIndex).text;

      // Strip comments
      const commentStart = lineText.indexOf(';');
      const codeText = commentStart !== -1 ? lineText.substring(0, commentStart) : lineText;

      // Definition line detection
      const isEquLine = /^\s*equ\s+/i.test(lineText);
      const isMemLine = /^\s*mem\s+/i.test(lineText);
      const isLabelLine = /^\s*[a-zA-Z_][a-zA-Z0-9_]*:/.test(lineText);

      combinedRegex.lastIndex = 0;
      let match;

      while ((match = combinedRegex.exec(codeText)) !== null) {
        const symbol = symbols.get(match[1].toLowerCase());
        if (!symbol) { continue; }

        // Determine token type and modifiers
        let tokenType = 0;
        let tokenModifiers = 0;

        switch (symbol.type) {
          case 'register':  tokenType = 0; break;
          case 'memory':    tokenType = 1; break;
          case 'label':     tokenType = 2; break;
          case 'constant':  tokenType = 3; tokenModifiers = 1; break;
        }

        // Declaration modifier
        if (lineIndex === symbol.line) {
          if ((isEquLine || isMemLine) && match.index === codeText.search(new RegExp(`\\b${this.escapeRegex(symbol.name)}\\b`, 'i'))) {
            tokenModifiers |= 1 << 0;
          }
          if (isLabelLine && new RegExp(`^\\s*${this.escapeRegex(symbol.name)}:`, 'i').test(lineText)) {
            tokenModifiers |= 1 << 0;
          }
        }

        tokensBuilder.push(lineIndex, match.index, match[1].length, tokenType, tokenModifiers);
      }
    }

    return tokensBuilder.build();
  }

  /**
   * @brief Parse the document to extract all symbol definitions
   * Keys are stored lowercase for case-insensitive matching.
   */
  private parseSymbols(document: vscode.TextDocument): Map<string, SpinASMSymbol> {
    const symbols = new Map<string, SpinASMSymbol>();
    const text = document.getText();

    // Parse EQU declarations: equ <name> <value>
    const equRegex = /^\s*equ\s+([a-zA-Z_][a-zA-Z0-9_]*)\s+(.+?)(?:;.*)?$/gmi;
    let match;

    while ((match = equRegex.exec(text)) !== null) {
      const symbolName = match[1];
      const symbolValue = match[2].trim();
      const isRegister = /^(?:reg\d+|adcl|adcr|dacl|dacr|pot[0-2])$/i.test(symbolValue);

      symbols.set(symbolName.toLowerCase(), {
        name: symbolName,
        type: isRegister ? 'register' : 'constant',
        value: symbolValue,
        line: this.getLineNumber(text, match.index)
      });
    }

    // Parse MEM declarations: mem <name> <size>
    const memRegex = /^\s*mem\s+([a-zA-Z_][a-zA-Z0-9_]*)\s+\d+/gmi;

    while ((match = memRegex.exec(text)) !== null) {
      const key = match[1].toLowerCase();
      symbols.set(key, {
        name: match[1],
        type: 'memory',
        line: this.getLineNumber(text, match.index)
      });
    }

    // Parse label definitions: <name>:
    const labelRegex = /^\s*([a-zA-Z_][a-zA-Z0-9_]*):/gm;

    while ((match = labelRegex.exec(text)) !== null) {
      const key = match[1].toLowerCase();
      if (!symbols.has(key)) {
        symbols.set(key, {
          name: match[1],
          type: 'label',
          line: this.getLineNumber(text, match.index)
        });
      }
    }

    return symbols;
  }

  /**
   * @brief Get line number from character index in document
   */
  private getLineNumber(text: string, index: number): number {
    return text.substring(0, index).split('\n').length - 1;
  }

  /**
   * @brief Escape special regex characters in a string
   */
  private escapeRegex(str: string): string {
    return str.replace(/[.*+?^${}()|[\]\\]/g, '\\$&');
  }
}

/**
 * @class SpinASMHoverProvider
 * @brief Provides hover information for SpinASM symbols
 *
 * Shows the original definition when hovering over a symbol.
 * Example: Hovering over 'mono' shows "equ mono reg0"
 */
export class SpinASMHoverProvider implements vscode.HoverProvider {

  provideHover(
    document: vscode.TextDocument,
    position: vscode.Position,
    token: vscode.CancellationToken
  ): vscode.ProviderResult<vscode.Hover> {

    const wordRange = document.getWordRangeAtPosition(position);
    if (!wordRange) {
      return;
    }

    const word = document.getText(wordRange);
    const symbols = this.parseSymbols(document);
    const symbol = symbols.get(word.toLowerCase());

    if (!symbol) {
      return;
    }

    // Build hover content
    let hoverText = '';

    switch (symbol.type) {
      case 'register':
        hoverText = `\`\`\`spinasm\nequ ${symbol.name} ${symbol.value}\n\`\`\`\n\nUser-defined register alias`;
        break;
      case 'memory':
        hoverText = `\`\`\`spinasm\nmem ${symbol.name}\n\`\`\`\n\nDelay memory buffer`;
        break;
      case 'label':
        hoverText = `\`\`\`spinasm\n${symbol.name}:\n\`\`\`\n\nJump label`;
        break;
      case 'constant':
        hoverText = `\`\`\`spinasm\nequ ${symbol.name} ${symbol.value}\n\`\`\`\n\nConstant value`;
        break;
    }

    const markdown = new vscode.MarkdownString(hoverText);
    return new vscode.Hover(markdown, wordRange);
  }

  /**
   * @brief Parse symbols (same logic as semantic tokens provider, lowercase keys)
   */
  private parseSymbols(document: vscode.TextDocument): Map<string, SpinASMSymbol> {
    const symbols = new Map<string, SpinASMSymbol>();
    const text = document.getText();

    const equRegex = /^\s*equ\s+([a-zA-Z_][a-zA-Z0-9_]*)\s+(.+?)(?:;.*)?$/gmi;
    let match;

    while ((match = equRegex.exec(text)) !== null) {
      const symbolName = match[1];
      const symbolValue = match[2].trim();
      const isRegister = /^(?:reg\d+|adcl|adcr|dacl|dacr|pot[0-2])$/i.test(symbolValue);

      symbols.set(symbolName.toLowerCase(), {
        name: symbolName,
        type: isRegister ? 'register' : 'constant',
        value: symbolValue,
        line: 0
      });
    }

    const memRegex = /^\s*mem\s+([a-zA-Z_][a-zA-Z0-9_]*)\s+\d+/gmi;

    while ((match = memRegex.exec(text)) !== null) {
      symbols.set(match[1].toLowerCase(), {
        name: match[1],
        type: 'memory',
        line: 0
      });
    }

    const labelRegex = /^\s*([a-zA-Z_][a-zA-Z0-9_]*):/gm;

    while ((match = labelRegex.exec(text)) !== null) {
      const key = match[1].toLowerCase();
      if (!symbols.has(key)) {
        symbols.set(key, {
          name: match[1],
          type: 'label',
          line: 0
        });
      }
    }

    return symbols;
  }
}
