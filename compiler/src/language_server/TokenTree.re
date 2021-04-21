open Kore;

type token_t =
  | Skip
  | Join
  | Identifier(Identifier.t)
  | Primitive(AST.raw_primitive_t);

type t = RangeTree.t(token_t);

let __skip: t = BinaryTree.create((Cursor.zero |> Cursor.expand, Skip));

let _join = (left: t, right: t) =>
  switch (left, right) {
  | ({value: (_, Skip)}, {value: (_, Skip)}) => __skip
  | (only, {value: (_, Skip)})
  | ({value: (_, Skip)}, only) => only
  | _ =>
    BinaryTree.create(
      ~left,
      ~right,
      ((left.value |> fst |> fst, right.value |> fst |> snd), Join),
    )
  };

let _fold = f => List.fold_left((acc, x) => _join(acc, f(x)), __skip);

let _wrap = (cursor, tree: t) =>
  cursor
  |> Cursor.expand
  |> (
    range =>
      tree.value |> fst == range
        ? tree : BinaryTree.create(~left=tree, (range, Join))
  );

let of_id = (id, cursor) =>
  BinaryTree.create((cursor |> Cursor.expand, Identifier(id)));

let rec of_list = (xs: list(t)): t =>
  switch (xs) {
  | [] => __skip
  | [x] => x
  | _ =>
    switch (xs |> List.divide |> Tuple.map2(of_list)) {
    | ({value: (_, Skip)}, {value: (_, Skip)}) => __skip
    | ({value: (_, Skip)}, only)
    | (only, {value: (_, Skip)}) => only
    | (head, tail) => _join(head, tail)
    }
  };

let rec of_expr =
  fun
  | AST.Primitive((prim, _, cursor)) =>
    BinaryTree.create((cursor |> Cursor.expand, Primitive(prim)))
  | AST.Identifier(id) => id |> Tuple.reduce2(of_id)
  | AST.JSX((jsx, cursor)) => of_jsx(jsx) |> _wrap(cursor)
  | AST.Group((expr, _, cursor)) => of_expr(expr) |> _wrap(cursor)
  | AST.BinaryOp(_, (left, _, l_cursor), (right, _, r_cursor)) =>
    _join(
      of_expr(left) |> _wrap(l_cursor),
      of_expr(right) |> _wrap(r_cursor),
    )
  | AST.UnaryOp(_, (expr, _, cursor)) => of_expr(expr) |> _wrap(cursor)
  | AST.Closure(stmts) => stmts |> List.map(of_stmt) |> of_list
and of_jsx =
  fun
  /* TODO: support jsx structures (just need to map to a tree) */
  | _ => __skip
and of_stmt =
  fun
  | AST.Variable(name, (expr, _, expr_cursor)) =>
    _join(
      name |> Tuple.reduce2(of_id),
      of_expr(expr) |> _wrap(expr_cursor),
    )
  | AST.Expression((expr, _, cursor)) => of_expr(expr) |> _wrap(cursor);

let of_decl =
  fun
  | AST.Constant((expr, _, cursor)) => of_expr(expr) |> _wrap(cursor);

let of_import =
  fun
  | AST.MainImport(id)
  | AST.NamedImport(id, None) => id |> Tuple.reduce2(of_id)
  | AST.NamedImport(id, Some(alias)) =>
    (id, alias) |> Tuple.map2(Tuple.reduce2(of_id)) |> Tuple.reduce2(_join);

let of_mod_stmt =
  fun
  | AST.Import(namespace, imports) => imports |> _fold(of_import)
  | AST.Declaration(MainExport(id) | NamedExport(id), decl) =>
    _join(id |> Tuple.reduce2(of_id), of_decl(decl));

let of_ast = (program: AST.program_t) => program |> _fold(of_mod_stmt);

let to_string = (tree: t) =>
  BinaryTree.to_string(
    (((start, end_), token)) =>
      Print.fmt(
        "%s %s",
        switch (token) {
        | Join => ""
        | Skip => "[skip]"
        | Identifier(id) => Identifier.to_string(id)
        | Primitive(prim) => Debug.print_prim(prim) |> Pretty.to_string
        },
        Cursor.Range(start, end_) |> Debug.print_cursor,
      )
      |> String.trim,
    tree,
  );
