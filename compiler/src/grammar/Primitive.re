open Kore;

let nil =
  Keyword.nil
  >|= Block.extend(block => {...block, type_: Type.K_Nil, value: AST.nil});

let boolean =
  Keyword.true_
  >|= Block.cursor
  >|= (cursor => Block.create(~type_=Type.K_Boolean, cursor, true))
  <|> (
    Keyword.false_
    >|= Block.cursor
    >|= (cursor => Block.create(~type_=Type.K_Boolean, cursor, false))
  )
  >== AST.of_bool;

let number = Number.parser >== AST.of_num;

let string = Type.K_String <@ M.string >== AST.of_string;

let parser = choice([nil, boolean, number, string]);
