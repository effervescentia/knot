open Kore;

let nil: primitive_parser_t =
  Keyword.nil >|= N.map(_ => AR.nil) >|= N.add_type(TR.(`Nil));

let boolean: primitive_parser_t =
  Keyword.true_
  >|= N.map(_ => AR.of_bool(true))
  >|= N.add_type(TR.(`Boolean))
  <|> (
    Keyword.false_
    >|= N.map(_ => AR.of_bool(false))
    >|= N.add_type(TR.(`Boolean))
  );

let number: primitive_parser_t = Number.parser >|= N.map(AR.of_num);

let string: primitive_parser_t =
  M.string >|= N.map(AR.of_string) >|= N.add_type(TR.(`String));

let parser: primitive_parser_t = choice([nil, boolean, number, string]);
