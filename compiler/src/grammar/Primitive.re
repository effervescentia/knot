open Kore;
open AST.Raw;
open Type.Raw;

let nil = Keyword.nil >|= Node.Raw.cursor >|= (cursor => (nil, cursor));

let boolean =
  Keyword.true_
  >|= Node.Raw.cursor
  >|= (cursor => (of_bool(true), cursor))
  <|> (
    Keyword.false_
    >|= Node.Raw.cursor
    >|= (cursor => (of_bool(false), cursor))
  );

let number = Number.parser >|= Tuple.map_fst2(of_num);

let string =
  M.string
  >|= (
    block => (block |> Node.Raw.value |> of_string, Node.Raw.cursor(block))
  );

let parser = choice([nil, boolean, number, string]);
