import { ExtensionContext, workspace } from 'vscode';
import {
  LanguageClient,
  LanguageClientOptions,
  ServerOptions
} from 'vscode-languageclient/node';

let client: LanguageClient;

const LSP_BINARY =
  '/Users/benteichman/development/knot/compiler/_esy/default/store/i/knot-94e894a7/bin/knotc.exe';

export function activate(_context: ExtensionContext) {
  const args = ['lsp', '--color', 'false'];
  const debugArgs = [...args, '--debug'];

  const serverOptions: ServerOptions = {
    run: {
      command: LSP_BINARY,
      args: debugArgs
    },
    debug: {
      command: LSP_BINARY,
      args: debugArgs
    }
  };

  const clientOptions: LanguageClientOptions = {
    documentSelector: [{ scheme: 'file', language: 'knot' }],
    synchronize: {
      fileEvents: workspace.createFileSystemWatcher('**/.knot.yml')
    }
  };

  client = new LanguageClient('knot', 'Knot', serverOptions, clientOptions);

  client.start();
}

export function deactivate(): Thenable<void> | undefined {
  if (!client) return undefined;

  return client.stop();
}
