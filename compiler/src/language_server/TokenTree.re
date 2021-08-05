open Kore;

type token_t =
  | Skip
  | Join
  | Identifier(Identifier.t)
  | Primitive(AST.raw_primitive_t);

type t = RangeTree.t(token_t);

let __skip: t = BinaryTree.create((Cursor.(expand(zero)), Skip));

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

let of_untyped_id = (id, cursor) =>
  BinaryTree.create((Cursor.expand(cursor), Identifier(id)));
let of_id = (id, _, cursor) => of_untyped_id(id, cursor);

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
  | AST.Primitive(prim) =>
    BinaryTree.create((
      prim |> Node.cursor |> Cursor.expand,
      Primitive(Node.value(prim)),
    ))
  | AST.Identifier(id) => id |> Tuple.reduce3(of_id)
  | AST.JSX(jsx) => jsx |> Node.value |> of_jsx |> _wrap(Node.cursor(jsx))
  | AST.Group(expr) =>
    expr |> Node.value |> of_expr |> _wrap(Node.cursor(expr))
  | AST.BinaryOp(_, lhs, rhs) =>
    _join(
      lhs |> Node.value |> of_expr |> _wrap(Node.cursor(lhs)),
      rhs |> Node.value |> of_expr |> _wrap(Node.cursor(rhs)),
    )
  | AST.UnaryOp(_, expr) =>
    expr |> Node.value |> of_expr |> _wrap(Node.cursor(expr))
  | AST.Closure(stmts) => stmts |> List.map(Node.value % of_stmt) |> of_list

and of_jsx =
  fun
  | AST.Fragment(children) =>
    children
    |> List.map(child =>
         child |> Node.value |> of_jsx_child |> _wrap(Node.cursor(child))
       )
    |> of_list

  | AST.Tag(id, attrs, children) =>
    [id |> Tuple.reduce2(of_untyped_id)]
    @ (
      attrs
      |> List.map(attr =>
           attr |> Node.value |> of_jsx_attr |> _wrap(Node.cursor(attr))
         )
    )
    @ (
      children
      |> List.map(child =>
           child |> Node.value |> of_jsx_child |> _wrap(Node.cursor(child))
         )
    )
    |> of_list

and of_jsx_child =
  fun
  | AST.Node(tag) => tag |> Node.value |> of_jsx |> _wrap(Node.cursor(tag))
  | AST.InlineExpression(expr) =>
    expr |> Node.value |> of_expr |> _wrap(Node.cursor(expr))
  | AST.Text(text) =>
    BinaryTree.create((
      text |> Node.cursor |> Cursor.expand,
      Primitive(String(Node.value(text))),
    ))

and of_jsx_attr =
  fun
  | AST.ID(id) => id |> Tuple.reduce2(of_untyped_id)
  | AST.Class(id, None)
  | AST.Property(id, None) => id |> Tuple.reduce2(of_untyped_id)
  | AST.Class(id, Some(expr))
  | AST.Property(id, Some(expr)) =>
    _join(
      id |> Tuple.reduce2(of_untyped_id),
      expr |> Node.value |> of_expr |> _wrap(Node.cursor(expr)),
    )

and of_stmt =
  fun
  | AST.Variable(name, expr) =>
    _join(
      name |> Tuple.reduce2(of_untyped_id),
      expr |> Node.value |> of_expr |> _wrap(Node.cursor(expr)),
    )
  | AST.Expression(expr) =>
    expr |> Node.value |> of_expr |> _wrap(Node.cursor(expr));

let of_decl =
  fun
  | AST.Constant(expr) =>
    expr |> Node.value |> of_expr |> _wrap(Node.cursor(expr))
  | AST.Function(args, expr) =>
    _join(
      args
      |> _fold(
           Node.value
           % (
             (AST.{name, default}) => {
               ...name |> Tuple.reduce2(of_untyped_id),
               right: default |?> Node.value % of_expr,
             }
           ),
         ),
      expr |> Node.value |> of_expr |> _wrap(Node.cursor(expr)),
    );

let of_import =
  fun
  | AST.MainImport(id)
  | AST.NamedImport(id, None) => id |> Tuple.reduce2(of_untyped_id)
  | AST.NamedImport(id, Some(alias)) =>
    (id, alias)
    |> Tuple.map2(Tuple.reduce2(of_untyped_id))
    |> Tuple.reduce2(_join);

let of_mod_stmt =
  fun
  | AST.Import(namespace, imports) =>
    imports |> _fold(Node.Raw.value % of_import)
  | AST.Declaration(MainExport(id) | NamedExport(id), decl) =>
    _join(id |> Tuple.reduce2(of_untyped_id), decl |> Node.value |> of_decl);

let of_ast = (program: AST.program_t) =>
  program |> _fold(Node.Raw.value % of_mod_stmt);

let to_string = (tree: t) =>
  BinaryTree.to_string((((start, end_), token)) =>
    Print.fmt(
      "%s %s",
      switch (token) {
      | Join => ""
      | Skip => "[skip]"
      | Identifier(id) => Identifier.to_string(id)
      | Primitive(prim) => AST.Debug.print_prim(prim) |> Pretty.to_string
      },
      Cursor.Range(start, end_) |> Cursor.to_string,
    )
    |> String.trim
  );
