open Kore;

let parser =
  M.identifier
  >|= Tuple.split2(Block.value % Reference.Identifier.of_string, Block.cursor);
