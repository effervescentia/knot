open Kore;
open ModuleAliases;

type token_t =
  | Skip
  | Join
  | Identifier(Identifier.t)
  | Primitive(A.primitive_t);

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

/* static */

let of_untyped_id = (id, range) =>
  BinaryTree.create((range, Identifier(id)));

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
  | (A.Primitive(prim), _, range) =>
    BinaryTree.create((range, Primitive(prim)))
  | (A.Identifier(id), _, range) => of_untyped_id(id, range)
  | (A.JSX(jsx), _, range) => jsx |> of_jsx |> _wrap(range)
  | (A.Group(expr), _, range) =>
    expr |> of_expr |> _wrap(N.get_range(expr))
  | (A.BinaryOp(_, lhs, rhs), _, range) =>
    _join(
      lhs |> of_expr |> _wrap(N.get_range(lhs)),
      rhs |> of_expr |> _wrap(N.get_range(rhs)),
    )
  | (A.UnaryOp(_, expr), _, range) =>
    expr |> of_expr |> _wrap(N.get_range(expr))
  | (A.Closure(stmts), _, range) =>
    stmts |> List.map(N.get_value % of_stmt) |> of_list
  | (A.DotAccess(expr, props), _, range) =>
    expr |> of_expr |> _wrap(N.get_range(expr))

and of_jsx =
  fun
  | A.Fragment(children) =>
    children
    |> List.map(child =>
         child |> of_jsx_child |> _wrap(NR.get_range(child))
       )
    |> of_list

  | A.Tag(id, attrs, children) =>
    [id |> Tuple.join2(of_untyped_id)]
    @ (
      attrs
      |> List.map(attr =>
           attr |> NR.get_value |> of_jsx_attr |> _wrap(NR.get_range(attr))
         )
    )
    @ (
      children
      |> List.map(child =>
           child |> of_jsx_child |> _wrap(NR.get_range(child))
         )
    )
    |> of_list

and of_jsx_child =
  fun
  | (A.Node(tag), range) => tag |> of_jsx |> _wrap(range)
  | (A.InlineExpression(expr), range) =>
    expr |> of_expr |> _wrap(N.get_range(expr))
  | (A.Text(text), range) =>
    BinaryTree.create((range, Primitive(String(text))))

and of_jsx_attr =
  fun
  | A.ID(id) => id |> Tuple.join2(of_untyped_id)
  | A.Class(id, None)
  | A.Property(id, None) => id |> Tuple.join2(of_untyped_id)
  | A.Class(id, Some(expr))
  | A.Property(id, Some(expr)) =>
    _join(
      id |> Tuple.join2(of_untyped_id),
      expr |> of_expr |> _wrap(N.get_range(expr)),
    )

and of_stmt =
  fun
  | A.Variable(name, expr) =>
    _join(
      name |> Tuple.join2(of_untyped_id),
      expr |> of_expr |> _wrap(N.get_range(expr)),
    )
  | A.Expression(expr) => expr |> of_expr |> _wrap(N.get_range(expr));

let of_decl =
  fun
  | A.Constant(expr) => expr |> of_expr |> _wrap(N.get_range(expr))
  | A.Function(args, expr) =>
    _join(
      args
      |> _fold(
           N.get_value
           % (
             (A.{name, default}) => {
               ...name |> Tuple.join2(of_untyped_id),
               right: default |?> of_expr,
             }
           ),
         ),
      expr |> of_expr |> _wrap(N.get_range(expr)),
    )
  | A.View(props, expr) =>
    _join(
      props
      |> _fold(
           N.get_value
           % (
             (A.{name, default}) => {
               ...name |> Tuple.join2(of_untyped_id),
               right: default |?> of_expr,
             }
           ),
         ),
      expr |> of_expr |> _wrap(N.get_range(expr)),
    );

let of_import =
  fun
  | A.MainImport(id)
  | A.NamedImport(id, None) => id |> Tuple.join2(of_untyped_id)
  | A.NamedImport(id, Some(alias)) =>
    (id, alias)
    |> Tuple.map2(Tuple.join2(of_untyped_id))
    |> Tuple.join2(_join);

let of_mod_stmt =
  fun
  | A.StandardImport(imports) =>
    imports
    |> _fold(
         NR.get_value
         % (
           fun
           | (id, None) => id |> Tuple.join2(of_untyped_id)
           | (id, Some(alias)) =>
             (id, alias)
             |> Tuple.map2(Tuple.join2(of_untyped_id))
             |> Tuple.join2(_join)
         ),
       )
  | A.Import(namespace, imports) =>
    imports |> _fold(NR.get_value % of_import)
  | A.Declaration(MainExport(id) | NamedExport(id), decl) =>
    _join(id |> Tuple.join2(of_untyped_id), decl |> N.get_value |> of_decl);

let of_ast = (program: A.program_t) =>
  program |> _fold(NR.get_value % of_mod_stmt);

/* pretty printing */

let pp: Fmt.t(t) =
  (ppf, tree: t) =>
    BinaryTree.pp(
      (ppf, ((start, end_), token)) =>
        Fmt.pf(
          ppf,
          "%s %s",
          switch (token) {
          | Join => ""
          | Skip => "[skip]"
          | Identifier(id) => id |> ~@Identifier.pp
          | Primitive(prim) => prim |> A.Dump.prim_to_string
          },
          Range.create(start, end_) |> ~@Range.pp,
        ),
      ppf,
      tree,
    );
