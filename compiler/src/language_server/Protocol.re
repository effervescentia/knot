open Kore;

type request_t('a) = {
  jsonrpc: string,
  id: int,
  method: string,
  params: 'a,
};

type notification_t('a) = {
  jsonrpc: string,
  method: string,
  params: 'a,
};

type text_document_t = {uri: string};

type position_t = {
  line: int,
  character: int,
};

type text_document_item_t = {
  uri: string,
  language_id: string,
  version: int,
  text: string,
};

type progress_token =
  | Int(int)
  | String(string);

module Error = {
  type t =
    /* LSP errors */
    | ServerNotInitialized
    | UnknownErrorCode
    | RequestFailed
    | ServerCancelled
    | ContentModified
    | RequestCancelled;

  let to_int =
    fun
    | ServerNotInitialized => (-32002)
    | UnknownErrorCode => (-32001)
    | RequestFailed => (-32803)
    | ServerCancelled => (-32802)
    | ContentModified => (-32801)
    | RequestCancelled => (-32800);

  let get_message =
    fun
    | ServerNotInitialized => "The initialize request must be sent first."
    | UnknownErrorCode => "An unknown error code was encountered."
    | RequestFailed => "The request did not complete processing successfully."
    | ServerCancelled => "This server is in the process of shutting down."
    | ContentModified => "The content was modified from its original version."
    | RequestCancelled => "The request was cancelled before it could complete processing.";
};
