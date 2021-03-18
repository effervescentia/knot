open Kore;

module ExpressionHash = {
  type t = AST.expression_t;

  let equal = (x, y) => x === y;
  let hash = Hashtbl.hash;
};
module ExpressionHashtbl = Hashtbl.Make(ExpressionHash);

let tests = [
  CursorTest.suite,
  FilenameTest.suite,
  FunctionalTest.suite,
  GraphTest.suite,
  HashtblTest.suite,
  InfixTest.suite,
  IntTest.suite,
  ListTest.suite,
  PrintTest.suite,
  StringTest.suite,
  TupleTest.suite,
  "ExpressionTable"
  >::: [
    "test"
    >: (
      () => {
        let tbl = ExpressionHashtbl.create(3);
        let key1 = AST.Primitive((AST.Nil, Type.K_Nil, Cursor.zero));
        let key2 = AST.Primitive((AST.Nil, Type.K_Nil, Cursor.zero));

        ExpressionHashtbl.add(tbl, key1, 1);
        ExpressionHashtbl.add(tbl, key2, 2);

        Assert.int(2, ExpressionHashtbl.length(tbl));
        Assert.int(1, ExpressionHashtbl.find(tbl, key1));
        Assert.int(2, ExpressionHashtbl.find(tbl, key2));
      }
    ),
  ],
];
