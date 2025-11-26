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

    // Parse the document to find all symbol definitions
    const symbols = this.parseSymbols(document);

    // Highlight all occurrences of these symbols
    for (let lineIndex = 0; lineIndex < document.lineCount; lineIndex++) {
      if (token.isCancellationRequested) {
        return;
      }

      const line = document.lineAt(lineIndex);
      this.tokenizeLine(line, lineIndex, symbols, tokensBuilder);
    }

    return tokensBuilder.build();
  }

  /**
   * @brief Parse the document to extract all symbol definitions
   */
  private parseSymbols(document: vscode.TextDocument): Map<string, SpinASMSymbol> {
    const symbols = new Map<string, SpinASMSymbol>();
    const text = document.getText();

    // Parse EQU declarations: equ <name> <value>
    // Example: equ mono reg0
    const equRegex = /^\s*equ\s+([a-zA-Z_][a-zA-Z0-9_]*)\s+(.+?)(?:;.*)?$/gm;
    let match;

    while ((match = equRegex.exec(text)) !== null) {
      const symbolName = match[1];
      const symbolValue = match[2].trim();

      // Determine if this is a register alias or constant
      const isRegister = /^(?:reg\d+|adcl|adcr|dacl|dacr|pot[0-2])$/i.test(symbolValue);

      symbols.set(symbolName, {
        name: symbolName,
        type: isRegister ? 'register' : 'constant',
        value: symbolValue,
        line: this.getLineNumber(text, match.index)
      });
    }

    // Parse MEM declarations: mem <name> <size>
    // Example: mem chodel 4096
    const memRegex = /^\s*mem\s+([a-zA-Z_][a-zA-Z0-9_]*)\s+\d+/gm;

    while ((match = memRegex.exec(text)) !== null) {
      const symbolName = match[1];

      symbols.set(symbolName, {
        name: symbolName,
        type: 'memory',
        line: this.getLineNumber(text, match.index)
      });
    }

    // Parse label definitions: <name>:
    // Example: LOOP:
    const labelRegex = /^\s*([a-zA-Z_][a-zA-Z0-9_]*):/gm;

    while ((match = labelRegex.exec(text)) !== null) {
      const symbolName = match[1];

      // Skip if already defined (equ/mem takes precedence)
      if (!symbols.has(symbolName)) {
        symbols.set(symbolName, {
          name: symbolName,
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
   * @brief Tokenize a single line, highlighting user-defined symbols
   */
  private tokenizeLine(
    line: vscode.TextLine,
    lineIndex: number,
    symbols: Map<string, SpinASMSymbol>,
    builder: vscode.SemanticTokensBuilder
  ): void {
    const lineText = line.text;

    // Check if this line is a symbol definition
    const isEquLine = /^\s*equ\s+/i.test(lineText);
    const isMemLine = /^\s*mem\s+/i.test(lineText);
    const isLabelLine = /^\s*[a-zA-Z_][a-zA-Z0-9_]*:/.test(lineText);

    // Highlight each symbol occurrence in this line
    for (const [symbolName, symbol] of symbols) {

      // Create regex to find this symbol (word boundaries)
      const symbolRegex = new RegExp(`\\b${this.escapeRegex(symbolName)}\\b`, 'g');
      let match;

      while ((match = symbolRegex.exec(lineText)) !== null) {
        // Skip if we're in a comment
        const commentStart = lineText.indexOf(';');
        if (commentStart !== -1 && match.index >= commentStart) {
          continue;
        }

        // Determine token type and modifiers
        let tokenType = 0;  // Default: variable
        let tokenModifiers = 0;

        switch (symbol.type) {
          case 'register':
            tokenType = 0;  // variable
            break;
          case 'memory':
            tokenType = 1;  // property
            break;
          case 'label':
            tokenType = 2;  // function
            break;
          case 'constant':
            tokenType = 3;  // parameter
            tokenModifiers = 1;  // readonly
            break;
        }

        // If this is the definition line, add 'declaration' modifier
        if (lineIndex === symbol.line) {
          if ((isEquLine || isMemLine) && match.index === lineText.indexOf(symbolName)) {
            tokenModifiers |= 1 << 0;  // Add 'declaration' modifier
          }
          if (isLabelLine && lineText.startsWith(symbolName + ':')) {
            tokenModifiers |= 1 << 0;  // Add 'declaration' modifier
          }
        }

        // Add the token
        builder.push(
          lineIndex,
          match.index,
          symbolName.length,
          tokenType,
          tokenModifiers
        );
      }
    }
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
    const symbol = symbols.get(word);

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
   * @brief Parse symbols (same logic as semantic tokens provider)
   */
  private parseSymbols(document: vscode.TextDocument): Map<string, SpinASMSymbol> {
    const symbols = new Map<string, SpinASMSymbol>();
    const text = document.getText();

    // Parse EQU declarations
    const equRegex = /^\s*equ\s+([a-zA-Z_][a-zA-Z0-9_]*)\s+(.+?)(?:;.*)?$/gm;
    let match;

    while ((match = equRegex.exec(text)) !== null) {
      const symbolName = match[1];
      const symbolValue = match[2].trim();
      const isRegister = /^(?:reg\d+|adcl|adcr|dacl|dacr|pot[0-2])$/i.test(symbolValue);

      symbols.set(symbolName, {
        name: symbolName,
        type: isRegister ? 'register' : 'constant',
        value: symbolValue,
        line: 0
      });
    }

    // Parse MEM declarations
    const memRegex = /^\s*mem\s+([a-zA-Z_][a-zA-Z0-9_]*)\s+\d+/gm;

    while ((match = memRegex.exec(text)) !== null) {
      symbols.set(match[1], {
        name: match[1],
        type: 'memory',
        line: 0
      });
    }

    // Parse labels
    const labelRegex = /^\s*([a-zA-Z_][a-zA-Z0-9_]*):/gm;

    while ((match = labelRegex.exec(text)) !== null) {
      if (!symbols.has(match[1])) {
        symbols.set(match[1], {
          name: match[1],
          type: 'label',
          line: 0
        });
      }
    }

    return symbols;
  }
}
