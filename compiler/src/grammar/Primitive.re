open Kore;
open AST.Raw.Util;

let nil =
  Keyword.nil
  >|= Block.cursor
  >|= (cursor => (nil, Type.K_Strong(K_Nil), cursor));

let boolean =
  Keyword.true_
  >|= Block.cursor
  >|= (cursor => (to_bool(true), Type.K_Strong(K_Boolean), cursor))
  <|> (
    Keyword.false_
    >|= Block.cursor
    >|= (cursor => (to_bool(false), Type.K_Strong(K_Boolean), cursor))
  );

let number = Number.parser >|= Tuple.map_fst3(to_num);

let string =
  M.string
  >|= (
    block => (
      block |> Block.value |> to_string,
      Type.K_Strong(K_String),
      Block.cursor(block),
    )
  );

let parser = choice([nil, boolean, number, string]);
