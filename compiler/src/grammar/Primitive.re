open Kore;

let nil =
  Keyword.nil >|= Block.cursor >|= (cursor => (AST.nil, Type.K_Nil, cursor));

let boolean =
  Keyword.true_
  >|= Block.cursor
  >|= (cursor => (true |> AST.of_bool, Type.K_Boolean, cursor))
  <|> (
    Keyword.false_
    >|= Block.cursor
    >|= (cursor => (false |> AST.of_bool, Type.K_Boolean, cursor))
  );

let number = Number.parser >|= Tuple.map_fst3(AST.of_num);

let string =
  M.string
  >|= (
    block => (
      block |> Block.value |> AST.of_string,
      Type.K_String,
      block |> Block.cursor,
    )
  );

let parser = choice([nil, boolean, number, string]);
