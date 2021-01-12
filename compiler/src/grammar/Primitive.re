open Kore;

let nil =
  Keyword.nil
  >|= Block.extend(block => {...block, type_: Type.K_Nil, value: AST.nil})
  |> M.lexeme;

let boolean =
  Keyword.true_
  >|= Block.cursor
  >|= (cursor => Block.create(~type_=Type.K_Boolean, cursor, true))
  <|> (
    Keyword.false_
    >|= Block.cursor
    >|= (cursor => Block.create(~type_=Type.K_Boolean, cursor, false))
  )
  >== AST.of_bool
  |> M.lexeme;

let number = Number.parser >|= Block.wrap >== AST.of_num;

let string = M.string >== AST.of_string >@ Type.K_String;

let parser = choice([nil, boolean, number, string]);
