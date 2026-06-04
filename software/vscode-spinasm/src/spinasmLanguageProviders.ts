import * as vscode from "vscode";
import { DocumentParser } from "./documentParser";
import { INSTRUCTIONS, BUILT_IN_SYMBOLS } from "./spinasmLanguage";
import { getInstructionDoc, getSymbolDoc } from "./fv1Reference";

/**
 * Go-to-definition for user-defined symbols (equ / mem / labels). Built-in
 * instructions and registers have no definition site, so they return nothing.
 * SpinASM programs are single-file per bank, so definitions are always local.
 */
export class SpinASMDefinitionProvider implements vscode.DefinitionProvider {
  provideDefinition(
    document: vscode.TextDocument,
    position: vscode.Position,
    _token: vscode.CancellationToken
  ): vscode.ProviderResult<vscode.Definition> {
    const wordRange = document.getWordRangeAtPosition(position);
    if (!wordRange) {
      return;
    }

    const word = document.getText(wordRange);
    const symbol = DocumentParser.get(document).symbols.get(word.toUpperCase());
    if (!symbol) {
      return;
    }

    const defRange = new vscode.Range(
      symbol.line,
      symbol.character,
      symbol.line,
      symbol.character + symbol.length
    );
    return new vscode.Location(document.uri, defRange);
  }
}

/**
 * Completion for instructions, built-in registers/flags, and the user's own
 * equ / mem / label symbols. The built-in set is static so it is built once;
 * user symbols are pulled fresh from the parser cache on each request.
 */
export class SpinASMCompletionProvider implements vscode.CompletionItemProvider {
  private readonly staticItems = buildStaticItems();

  provideCompletionItems(
    document: vscode.TextDocument,
    position: vscode.Position,
    _token: vscode.CancellationToken
  ): vscode.ProviderResult<vscode.CompletionItem[]> {
    // Don't suggest inside comments.
    const linePrefix = document.lineAt(position.line).text.substring(0, position.character);
    if (linePrefix.includes(";")) {
      return;
    }

    const userItems: vscode.CompletionItem[] = [];
    for (const symbol of DocumentParser.get(document).symbols.values()) {
      const item = new vscode.CompletionItem(symbol.name, kindForSymbolType(symbol.type));
      item.detail = `SpinASM ${symbol.type}`;
      userItems.push(item);
    }

    return [...this.staticItems, ...userItems];
  }
}

function kindForSymbolType(type: string): vscode.CompletionItemKind {
  switch (type) {
    case "label":  return vscode.CompletionItemKind.Function;
    case "memory": return vscode.CompletionItemKind.Field;
    default:       return vscode.CompletionItemKind.Variable; // register / constant
  }
}

function buildStaticItems(): vscode.CompletionItem[] {
  const items: vscode.CompletionItem[] = [];

  for (const name of INSTRUCTIONS) {
    const item = new vscode.CompletionItem(name, vscode.CompletionItemKind.Keyword);
    const doc = getInstructionDoc(name);
    if (doc) {
      item.detail = doc.signature;
      item.documentation = new vscode.MarkdownString(doc.summary);
    }
    items.push(item);
  }

  for (const name of BUILT_IN_SYMBOLS) {
    // SOF/RDA live in both sets; their instruction item already covers them.
    if (INSTRUCTIONS.has(name)) {
      continue;
    }

    const item = new vscode.CompletionItem(name, vscode.CompletionItemKind.Variable);
    const doc = getSymbolDoc(name);
    if (doc) {
      item.detail = doc.signature;
      item.documentation = new vscode.MarkdownString(doc.summary);
    }
    items.push(item);
  }

  return items;
}
