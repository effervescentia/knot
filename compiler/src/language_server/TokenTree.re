open Kore;
open ModuleAliases;

type token_t =
  | Skip
  | Join
  | Identifier(string)
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

let of_untyped_id = ((id, (_, range))) =>
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
  | (A.Primitive(prim), (_, range)) =>
    BinaryTree.create((range, Primitive(prim)))
  | (A.Identifier(id), (_, range)) => N.untyped(id, range) |> of_untyped_id
  | (A.JSX(jsx), (_, range)) => jsx |> of_jsx |> _wrap(range)
  | (A.Group(expr), (_, range)) =>
    expr |> of_expr |> _wrap(N.get_range(expr))
  | (A.BinaryOp(_, lhs, rhs), (_, range)) =>
    _join(
      lhs |> of_expr |> _wrap(N.get_range(lhs)),
      rhs |> of_expr |> _wrap(N.get_range(rhs)),
    )
  | (A.UnaryOp(_, expr), _) => expr |> of_expr |> _wrap(N.get_range(expr))
  | (A.Closure(stmts), _) => stmts |> List.map(fst % of_stmt) |> of_list
  | (A.DotAccess(expr, props), _) =>
    expr |> of_expr |> _wrap(N.get_range(expr))
  | (A.FunctionCall(expr, args), (_, range)) =>
    _join(
      expr |> of_expr |> _wrap(N.get_range(expr)),
      args |> List.map(of_expr) |> of_list,
    )

and of_jsx =
  fun
  | A.Fragment(children) =>
    children
    |> List.map(child => child |> of_jsx_child |> _wrap(N.get_range(child)))
    |> of_list

  | A.Tag((id, (_, range)), attrs, children)
  | A.Component((id, (_, range)), attrs, children) =>
    [N.untyped(id, range) |> of_untyped_id]
    @ (
      attrs
      |> List.map(attr =>
           attr |> fst |> of_jsx_attr |> _wrap(N.get_range(attr))
         )
    )
    @ (
      children
      |> List.map(child =>
           child |> of_jsx_child |> _wrap(N.get_range(child))
         )
    )
    |> of_list

and of_jsx_child =
  fun
  | (A.Node(tag), (_, range)) => tag |> of_jsx |> _wrap(range)
  | (A.InlineExpression(expr), (_, range)) =>
    expr |> of_expr |> _wrap(N.get_range(expr))
  | (A.Text(text), (_, range)) =>
    BinaryTree.create((range, Primitive(String(text))))

and of_jsx_attr =
  fun
  | A.ID(id) => of_untyped_id(id)
  | A.Class(id, None)
  | A.Property(id, None) => of_untyped_id(id)
  | A.Class(id, Some(expr))
  | A.Property(id, Some(expr)) =>
    _join(of_untyped_id(id), expr |> of_expr |> _wrap(N.get_range(expr)))

and of_stmt =
  fun
  | A.Variable(name, expr) =>
    _join(of_untyped_id(name), expr |> of_expr |> _wrap(N.get_range(expr)))
  | A.Expression(expr) => expr |> of_expr |> _wrap(N.get_range(expr));

let of_args = args =>
  args
  |> _fold(
       fst
       % (
         (A.{name, default}) => {
           ...of_untyped_id(name),
           right: default |?> of_expr,
         }
       ),
     );

let of_decl =
  fun
  | A.Constant(expr) => expr |> of_expr |> _wrap(N.get_range(expr))
  | A.Enumerated(variants) =>
    variants
    |> List.map(((name, args)) => [of_untyped_id(name)])
    |> List.flatten
    |> of_list
  | A.Function(args, expr) =>
    _join(of_args(args), expr |> of_expr |> _wrap(N.get_range(expr)))
  | A.View(props, expr) =>
    _join(of_args(props), expr |> of_expr |> _wrap(N.get_range(expr)))
  | A.Style(args, rule_sets) =>
    _join(
      of_args(args),
      rule_sets
      |> List.map(fst % snd % List.map(fst % snd % of_expr))
      |> List.flatten
      |> of_list,
    );

let of_import =
  fun
  | A.MainImport(id)
  | A.NamedImport(id, None) => of_untyped_id(id)
  | A.NamedImport(id, Some(alias)) =>
    (id, alias) |> Tuple.map2(of_untyped_id) |> Tuple.join2(_join);

let of_mod_stmt =
  fun
  | A.StandardImport(imports) =>
    imports
    |> _fold(
         fst
         % (
           fun
           | (id, None) => of_untyped_id(id)
           | (id, Some(alias)) =>
             (id, alias) |> Tuple.map2(of_untyped_id) |> Tuple.join2(_join)
         ),
       )
  | A.Import(namespace, imports) => imports |> _fold(fst % of_import)
  | A.Declaration(MainExport(id) | NamedExport(id), (decl, _)) =>
    _join(of_untyped_id(id), of_decl(decl));

let of_ast = (program: A.program_t) => program |> _fold(fst % of_mod_stmt);

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
          | Identifier(id) => id
          | Primitive(prim) => prim |> A.Dump.prim_to_string
          },
          Range.create(start, end_) |> ~@Range.pp,
        ),
      ppf,
      tree,
    );
