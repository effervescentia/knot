open Kore;
open AST.Raw;
open Type.Raw;

let nil = Keyword.nil >|= Block.cursor >|= (cursor => (nil, cursor));

let boolean =
  Keyword.true_
  >|= Block.cursor
  >|= (cursor => (of_bool(true), cursor))
  <|> (
    Keyword.false_ >|= Block.cursor >|= (cursor => (of_bool(false), cursor))
  );

let number = Number.parser >|= Tuple.map_fst2(of_num);

let string =
  M.string
  >|= (block => (block |> Block.value |> of_string, Block.cursor(block)));

let parser = choice([nil, boolean, number, string]);
