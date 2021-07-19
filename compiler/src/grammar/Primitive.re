open Kore;

let nil =
  Keyword.nil
  >|= Block.cursor
  >|= (cursor => (RawUtil.nil, Type.K_Strong(K_Nil), cursor));

let boolean =
  Keyword.true_
  >|= Block.cursor
  >|= (cursor => (true |> RawUtil.bool, Type.K_Strong(K_Boolean), cursor))
  <|> (
    Keyword.false_
    >|= Block.cursor
    >|= (cursor => (false |> RawUtil.bool, Type.K_Strong(K_Boolean), cursor))
  );

let number = Number.parser >|= Tuple.map_fst3(RawUtil.num);

let string =
  M.string
  >|= (
    block => (
      block |> Block.value |> RawUtil.string,
      Type.K_Strong(K_String),
      block |> Block.cursor,
    )
  );

let parser = choice([nil, boolean, number, string]);
