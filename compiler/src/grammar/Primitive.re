open Kore;

let nil: primitive_parser_t =
  Keyword.nil >|= N2.map(_ => AR.nil) >|= N2.add_type(TR.(`Nil));

let boolean: primitive_parser_t =
  Keyword.true_
  >|= N2.map(_ => AR.of_bool(true))
  >|= N2.add_type(TR.(`Boolean))
  <|> (
    Keyword.false_
    >|= N2.map(_ => AR.of_bool(false))
    >|= N2.add_type(TR.(`Boolean))
  );

let number: primitive_parser_t = Number.parser >|= N2.map(AR.of_num);

let string: primitive_parser_t =
  M.string >|= N2.map(AR.of_string) >|= N2.add_type(TR.(`String));

let parser: primitive_parser_t = choice([nil, boolean, number, string]);
