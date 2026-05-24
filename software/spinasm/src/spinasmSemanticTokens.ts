import * as vscode from 'vscode';
import { DocumentParser } from './documentParser';

/** Semantic highlighting for user-defined SpinASM symbols (equ, mem, labels). */
export class SpinASMSemanticTokensProvider implements vscode.DocumentSemanticTokensProvider {

  static readonly legend = new vscode.SemanticTokensLegend(
    [
      'variable',      // 0: equ register aliases
      'property',      // 1: mem buffer names
      'function',      // 2: labels
      'parameter'      // 3: constants
    ],
    [
      'declaration',   // 0: definition site
      'readonly'       // 1: constant values
    ]
  );

  // Yield every CHUNK_SIZE lines so large files don't block the extension host.
  private static readonly CHUNK_SIZE = 200;

  private static readonly WORD_REGEX = /\b[a-zA-Z_]\w*\b/g;

  async provideDocumentSemanticTokens(
    document: vscode.TextDocument,
    token: vscode.CancellationToken
  ): Promise<vscode.SemanticTokens | undefined> {

    const tokensBuilder = new vscode.SemanticTokensBuilder(
      SpinASMSemanticTokensProvider.legend
    );

    const parsedDoc = DocumentParser.get(document);
    const symbols = parsedDoc.symbols;

    if (symbols.size === 0) {
      return tokensBuilder.build();
    }

    const wordRegex = SpinASMSemanticTokensProvider.WORD_REGEX;
    const chunkSize = SpinASMSemanticTokensProvider.CHUNK_SIZE;

    for (let lineIndex = 0; lineIndex < document.lineCount; lineIndex++) {
      if (token.isCancellationRequested) {
        return;
      }

      if (lineIndex > 0 && lineIndex % chunkSize === 0) {
        await new Promise<void>(resolve => setImmediate(resolve));
        if (token.isCancellationRequested) {
          return;
        }
      }

      const lineText = document.lineAt(lineIndex).text;

      const commentStart = lineText.indexOf(';');
      const codeText = commentStart !== -1 ? lineText.substring(0, commentStart) : lineText;

      wordRegex.lastIndex = 0;
      let match;

      while ((match = wordRegex.exec(codeText)) !== null) {
        const word = match[0];
        const symbol = symbols.get(word.toUpperCase());
        if (!symbol) { continue; }

        let tokenType = 0;
        let tokenModifiers = 0;

        switch (symbol.type) {
          case 'register':  tokenType = 0; break;
          case 'memory':    tokenType = 1; break;
          case 'label':     tokenType = 2; break;
          case 'constant':  tokenType = 3; tokenModifiers = 1; break;
        }

        if (lineIndex === symbol.line && match.index === symbol.character) {
          tokenModifiers |= 1 << 0;
        }

        tokensBuilder.push(lineIndex, match.index, word.length, tokenType, tokenModifiers);
      }
    }

    return tokensBuilder.build();
  }
}

/** Hover popups for SpinASM symbols, served from the DocumentParser cache. */
export class SpinASMHoverProvider implements vscode.HoverProvider {

  provideHover(
    document: vscode.TextDocument,
    position: vscode.Position,
    _token: vscode.CancellationToken
  ): vscode.ProviderResult<vscode.Hover> {

    const wordRange = document.getWordRangeAtPosition(position);
    if (!wordRange) {
      return;
    }

    const word = document.getText(wordRange);
    const parsedDoc = DocumentParser.get(document);
    const symbol = parsedDoc.symbols.get(word.toUpperCase());

    if (!symbol) {
      return;
    }

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
