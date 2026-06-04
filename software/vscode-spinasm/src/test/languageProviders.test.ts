import * as assert from "assert";
import * as vscode from "vscode";

async function openSpn(content: string): Promise<vscode.TextDocument> {
  const doc = await vscode.workspace.openTextDocument({ content, language: "spinasm" });
  await vscode.window.showTextDocument(doc);
  return doc;
}

function hoverText(hovers: vscode.Hover[] | undefined): string {
  if (!hovers) {
    return "";
  }
  return hovers
    .flatMap(h => h.contents.map(c => (typeof c === "string" ? c : (c as vscode.MarkdownString).value)))
    .join("\n");
}

describe("language providers (integration)", () => {
  // Make sure the extension is active so its providers are registered.
  before(async function () {
    this.timeout(20000);
    const ext = vscode.extensions.all.find(e => e.packageJSON && e.packageJSON.name === "spinasm");
    if (ext && !ext.isActive) {
      await ext.activate();
    }
  });

  it("hover shows built-in instruction docs for RDAX", async () => {
    const doc = await openSpn("rdax adcl, 1.0\n");
    const hovers = await vscode.commands.executeCommand<vscode.Hover[]>(
      "vscode.executeHoverProvider", doc.uri, new vscode.Position(0, 1)
    );
    assert.match(hoverText(hovers), /ACC/);
  });

  it("hover shows built-in register docs for ADCL", async () => {
    const doc = await openSpn("rdax adcl, 1.0\n");
    const hovers = await vscode.commands.executeCommand<vscode.Hover[]>(
      "vscode.executeHoverProvider", doc.uri, new vscode.Position(0, 6)
    );
    assert.match(hoverText(hovers), /ADC/i);
  });

  it("go-to-definition jumps to an equ definition", async () => {
    const doc = await openSpn("equ vol reg0\nrdax vol, 1.0\n");
    const locations = await vscode.commands.executeCommand<vscode.Location[]>(
      "vscode.executeDefinitionProvider", doc.uri, new vscode.Position(1, 5)
    );
    assert.ok(locations && locations.length > 0, "expected a definition location");
    assert.strictEqual(locations[0].range.start.line, 0);
  });

  it("completion offers instructions and built-in registers", async () => {
    const doc = await openSpn("\n");
    const list = await vscode.commands.executeCommand<vscode.CompletionList>(
      "vscode.executeCompletionItemProvider", doc.uri, new vscode.Position(0, 0)
    );
    const labels = list.items.map(i => (typeof i.label === "string" ? i.label : i.label.label));
    assert.ok(labels.includes("RDAX"), "instructions should be offered");
    assert.ok(labels.includes("ADCL"), "built-in registers should be offered");
  });

  it("completion offers user-defined symbols", async () => {
    const doc = await openSpn("equ tempo 32\nmem delay 4096\n");
    const list = await vscode.commands.executeCommand<vscode.CompletionList>(
      "vscode.executeCompletionItemProvider", doc.uri, new vscode.Position(1, 0)
    );
    const labels = list.items.map(i => (typeof i.label === "string" ? i.label : i.label.label));
    assert.ok(labels.includes("tempo"), "user constants should be offered");
    assert.ok(labels.includes("delay"), "user mem buffers should be offered");
  });
});
