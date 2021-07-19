open Kore;

let nil = Keyword.nil >|= Block.cursor >|= (cursor => (Type2.Nil, cursor));

let bool =
  Keyword.bool >|= Block.cursor >|= (cursor => (Type2.Boolean, cursor));

let int =
  Keyword.int >|= Block.cursor >|= (cursor => (Type2.Integer, cursor));

let float =
  Keyword.float >|= Block.cursor >|= (cursor => (Type2.Float, cursor));

let number =
  Keyword.number
  >|= Block.cursor
  >|= (cursor => (Type2.Abstract(Numeric), cursor));

let string =
  Keyword.string >|= Block.cursor >|= (cursor => (Type2.String, cursor));

let element =
  Keyword.element >|= Block.cursor >|= (cursor => (Type2.Element, cursor));

let parser = choice([nil, bool, int, float, number, string, element]);
