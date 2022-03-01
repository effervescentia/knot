open Kore;

let nil: primitive_parser_t =
  Keyword.nil >|= NR.map_value(_ => AR.nil) >|= N.of_raw(TR.(`Nil));

let boolean: primitive_parser_t =
  Keyword.true_
  >|= NR.map_value(_ => AR.of_bool(true))
  >|= N.of_raw(TR.(`Boolean))
  <|> (
    Keyword.false_
    >|= NR.map_value(_ => AR.of_bool(false))
    >|= N.of_raw(TR.(`Boolean))
  );

let number: primitive_parser_t = Number.parser >|= N.map_value(AR.of_num);

let string: primitive_parser_t =
  M.string >|= NR.map_value(AR.of_string) >|= N.of_raw(TR.(`String));

let parser: primitive_parser_t = choice([nil, boolean, number, string]);
