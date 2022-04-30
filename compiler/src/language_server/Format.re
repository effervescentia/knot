open Kore;
open Deserialize;

type formatting_options_t = {
  tab_size: int,
  insert_spaces: bool,
  trim_trailing_whitespace: option(bool),
  insert_final_newline: option(bool),
  trim_final_newlines: option(bool),
};

type params_t = {
  text_document: text_document_t,
  options: formatting_options_t,
  partial_result_token: option(progress_token),
};

let method_key = "textDocument/formatting";

let deserialize =
  JSON.Util.(
    json => {
      let text_document = get_text_document(json);
      let options =
        json
        |> member("options")
        |> (
          x => {
            let tab_size = x |> member("tabSize") |> to_int;
            let insert_spaces = x |> member("insertSpaces") |> to_bool;
            let trim_trailing_whitespace =
              x |> member("trimTrailingWhitespace") |> to_bool_option;
            let insert_final_newline =
              x |> member("insertFinalNewline") |> to_bool_option;
            let trim_final_newlines =
              x |> member("trimFinalNewlines") |> to_bool_option;

            {
              tab_size,
              insert_spaces,
              trim_trailing_whitespace,
              insert_final_newline,
              trim_final_newlines,
            };
          }
        );

      {text_document, options, partial_result_token: None};
    }
  );

let handler: Runtime.request_handler_t(params_t) =
  (runtime, {text_document: {uri}}) => Result.ok(`Null);
