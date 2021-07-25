open Kore;
open AST.Raw;
open Type2.Raw;

let nil =
  Keyword.nil >|= Block.cursor >|= (cursor => (nil, Strong(`Nil), cursor));

let boolean =
  Keyword.true_
  >|= Block.cursor
  >|= (cursor => (of_bool(true), Strong(`Boolean), cursor))
  <|> (
    Keyword.false_
    >|= Block.cursor
    >|= (cursor => (of_bool(false), Strong(`Boolean), cursor))
  );

let number = Number.parser >|= Tuple.map_fst3(of_num);

let string =
  M.string
  >|= (
    block => (
      block |> Block.value |> of_string,
      Strong(`String),
      Block.cursor(block),
    )
  );

let parser = choice([nil, boolean, number, string]);
