type DocumentUri = string;

export type TraceValue = 'off' | 'messages' | 'verbose';

export interface ClientCapabilities {
  workspace?: {};
}

export interface WorkspaceFolder {
  uri: DocumentUri;
  name: string;
}

export interface InitializeRequest {
  processId: number | null;

  clientInfo?: {
    name: string;
    version?: string;
  };

  locale?: string;

  rootUri: DocumentUri | null;

  initializationOptions?: any;

  capabilities: ClientCapabilities;

  trace?: TraceValue;

  workspaceFolders?: WorkspaceFolder[] | null;
}
