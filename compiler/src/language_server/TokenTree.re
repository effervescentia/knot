open Kore;

type token_t =
  | Skip
  | Join
  | Identifier(Identifier.t)
  | Primitive(AST.raw_primitive_t);

type t = RangeTree.t(token_t);

let __skip: t = BinaryTree.create((Range.zero, Skip));

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

let _wrap = (range, tree: t) =>
  tree.value |> fst == range
    ? tree : BinaryTree.create(~left=tree, (range, Join));

let of_untyped_id = (id, range) =>
  BinaryTree.create((range, Identifier(id)));
let of_id = (id, _, range) => of_untyped_id(id, range);

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
      prim |> Node.get_range,
      Primitive(Node.get_value(prim)),
    ))
  | AST.Identifier(id) => id |> Tuple.join3(of_id)
  | AST.JSX(jsx) =>
    jsx |> Node.get_value |> of_jsx |> _wrap(Node.get_range(jsx))
  | AST.Group(expr) =>
    expr |> Node.get_value |> of_expr |> _wrap(Node.get_range(expr))
  | AST.BinaryOp(_, lhs, rhs) =>
    _join(
      lhs |> Node.get_value |> of_expr |> _wrap(Node.get_range(lhs)),
      rhs |> Node.get_value |> of_expr |> _wrap(Node.get_range(rhs)),
    )
  | AST.UnaryOp(_, expr) =>
    expr |> Node.get_value |> of_expr |> _wrap(Node.get_range(expr))
  | AST.Closure(stmts) =>
    stmts |> List.map(Node.get_value % of_stmt) |> of_list

and of_jsx =
  fun
  | AST.Fragment(children) =>
    children
    |> List.map(child =>
         child
         |> Node.get_value
         |> of_jsx_child
         |> _wrap(Node.get_range(child))
       )
    |> of_list

  | AST.Tag(id, attrs, children) =>
    [id |> Tuple.join2(of_untyped_id)]
    @ (
      attrs
      |> List.map(attr =>
           attr
           |> Node.get_value
           |> of_jsx_attr
           |> _wrap(Node.get_range(attr))
         )
    )
    @ (
      children
      |> List.map(child =>
           child
           |> Node.get_value
           |> of_jsx_child
           |> _wrap(Node.get_range(child))
         )
    )
    |> of_list

and of_jsx_child =
  fun
  | AST.Node(tag) =>
    tag |> Node.get_value |> of_jsx |> _wrap(Node.get_range(tag))
  | AST.InlineExpression(expr) =>
    expr |> Node.get_value |> of_expr |> _wrap(Node.get_range(expr))
  | AST.Text(text) =>
    BinaryTree.create((
      Node.get_range(text),
      Primitive(String(Node.get_value(text))),
    ))

and of_jsx_attr =
  fun
  | AST.ID(id) => id |> Tuple.join2(of_untyped_id)
  | AST.Class(id, None)
  | AST.Property(id, None) => id |> Tuple.join2(of_untyped_id)
  | AST.Class(id, Some(expr))
  | AST.Property(id, Some(expr)) =>
    _join(
      id |> Tuple.join2(of_untyped_id),
      expr |> Node.get_value |> of_expr |> _wrap(Node.get_range(expr)),
    )

and of_stmt =
  fun
  | AST.Variable(name, expr) =>
    _join(
      name |> Tuple.join2(of_untyped_id),
      expr |> Node.get_value |> of_expr |> _wrap(Node.get_range(expr)),
    )
  | AST.Expression(expr) =>
    expr |> Node.get_value |> of_expr |> _wrap(Node.get_range(expr));

let of_decl =
  fun
  | AST.Constant(expr) =>
    expr |> Node.get_value |> of_expr |> _wrap(Node.get_range(expr))
  | AST.Function(args, expr) =>
    _join(
      args
      |> _fold(
           Node.get_value
           % (
             (AST.{name, default}) => {
               ...name |> Tuple.join2(of_untyped_id),
               right: default |?> Node.get_value % of_expr,
             }
           ),
         ),
      expr |> Node.get_value |> of_expr |> _wrap(Node.get_range(expr)),
    );

let of_import =
  fun
  | AST.MainImport(id)
  | AST.NamedImport(id, None) => id |> Tuple.join2(of_untyped_id)
  | AST.NamedImport(id, Some(alias)) =>
    (id, alias)
    |> Tuple.map2(Tuple.join2(of_untyped_id))
    |> Tuple.join2(_join);

let of_mod_stmt =
  fun
  | AST.Import(namespace, imports) =>
    imports |> _fold(Node.Raw.get_value % of_import)
  | AST.Declaration(MainExport(id) | NamedExport(id), decl) =>
    _join(
      id |> Tuple.join2(of_untyped_id),
      decl |> Node.get_value |> of_decl,
    );

let of_ast = (program: AST.program_t) =>
  program |> _fold(Node.Raw.get_value % of_mod_stmt);

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
      Range.create(start, end_) |> Range.to_string,
    )
    |> String.trim
  );
