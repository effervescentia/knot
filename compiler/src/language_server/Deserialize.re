open Kore;
open JSON.Util;

let position =
  member("position")
  % (
    fun
    | `Assoc(_) as x => {
        let line = (x |> member("line") |> to_int) + 1;
        let character = (x |> member("character") |> to_int) + 1;

        Protocol.{line, character};
      }
    | x => raise(Type_error("position", x))
  );

let uri = member("uri") % to_string % String.drop_prefix(file_schema);

let text_document =
  member("textDocument")
  % (
    fun
    | `Assoc(_) as x => {
        let uri_value = uri(x);

        Protocol.{uri: uri_value};
      }
    | x => raise(Type_error("textDocument", x))
  );

let text_document_item =
  member("textDocument")
  % (
    fun
    | `Assoc(_) as x => {
        let uri = uri(x);
        let language_id = x |> member("languageId") |> to_string;
        let version = x |> member("version") |> to_int;
        let text = x |> member("text") |> to_string;

        Protocol.{uri, language_id, version, text};
      }
    | x => raise(Type_error("textDocument", x))
  );
