import * as vscode from 'vscode';
import { DocumentParser } from './documentParser';

/**
 * @class SpinASMSemanticTokensProvider
 * @brief Provides semantic highlighting for user-defined SpinASM symbols
 * Retrieves symbols directly from the DocumentParser cache for instant performance.
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

    // Get the unified AST model parsed exactly once
    const parsedDoc = DocumentParser.get(document);
    const symbols = parsedDoc.symbols;

    if (symbols.size === 0) {
      return tokensBuilder.build();
    }

    // Build a single combined regex for all symbols (case-insensitive) using original defined names
    const escapedNames = Array.from(symbols.values()).map(sym => this.escapeRegex(sym.name));
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

      combinedRegex.lastIndex = 0;
      let match;

      while ((match = combinedRegex.exec(codeText)) !== null) {
        // Retrieve symbol using UPPERCASE key to match DocumentParser map
        const symbol = symbols.get(match[1].toUpperCase());
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

        // Direct, zero-allocation O(1) declaration checking via cached offsets
        if (lineIndex === symbol.line && match.index === symbol.character) {
          tokenModifiers |= 1 << 0;
        }

        tokensBuilder.push(lineIndex, match.index, match[1].length, tokenType, tokenModifiers);
      }
    }

    return tokensBuilder.build();
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
 * Uses the cached unified Parser mapping for fast 0ms hover popups.
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
    const parsedDoc = DocumentParser.get(document);
    // Retrieve symbol using UPPERCASE key to match DocumentParser map
    const symbol = parsedDoc.symbols.get(word.toUpperCase());

    if (!symbol) {
      return;
    }

    // Build hover markdown content
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
}
