open Kore;

let nil =
  Keyword.nil
  >|= Block.cursor
  >|= (cursor => (AST.Raw.nil, Type2.Raw.Strong(`Nil), cursor));

let boolean =
  Keyword.true_
  >|= Block.cursor
  >|= (
    cursor => (true |> AST.Raw.of_bool, Type2.Raw.Strong(`Boolean), cursor)
  )
  <|> (
    Keyword.false_
    >|= Block.cursor
    >|= (
      cursor => (
        false |> AST.Raw.of_bool,
        Type2.Raw.Strong(`Boolean),
        cursor,
      )
    )
  );

let number = Number.parser >|= Tuple.map_fst3(AST.Raw.of_num);

let string =
  M.string
  >|= (
    block => (
      block |> Block.value |> AST.Raw.of_string,
      Type2.Raw.Strong(`String),
      block |> Block.cursor,
    )
  );

let parser = choice([nil, boolean, number, string]);
