open Kore;
open AST.Raw;
open Type.Raw;

let nil = Keyword.nil >|= Node.Raw.get_range >|= (range => (nil, range));

let boolean =
  Keyword.true_
  >|= Node.Raw.get_range
  >|= (range => (of_bool(true), range))
  <|> (
    Keyword.false_
    >|= Node.Raw.get_range
    >|= (range => (of_bool(false), range))
  );

let number = Number.parser >|= Tuple.map_fst2(of_num);

let string =
  M.string
  >|= (node => Node.Raw.(node |> get_value |> of_string, get_range(node)));

let parser = choice([nil, boolean, number, string]);
