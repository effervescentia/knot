open Kore;
open ModuleAliases;

module A = AST.Result;
module AE = AST.Expression;

type token_t =
  | Skip
  | Join
  | Identifier(string)
  | Primitive(A.primitive_t);

type t = RangeTree.t(token_t);

let __skip: t = BinaryTree.create((Range.zero, Skip));

let _join = (left: t, right: t) =>
  switch (left, right) {
  | ({value: (_, Skip), _}, {value: (_, Skip), _}) => __skip
  | (only, {value: (_, Skip), _})
  | ({value: (_, Skip), _}, only) => only
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
    | ({value: (_, Skip), _}, {value: (_, Skip), _}) => __skip
    | ({value: (_, Skip), _}, only)
    | (only, {value: (_, Skip), _}) => only
    | (head, tail) => _join(head, tail)
    }
  };

let rec of_expr =
  fun
  | (AE.Primitive(prim), (_, range)) =>
    BinaryTree.create((range, Primitive(prim)))
  | (AE.Identifier(id), (_, range)) => N.untyped(id, range) |> of_untyped_id
  | (AE.JSX(jsx), (_, range)) => jsx |> of_jsx |> _wrap(range)
  | (AE.Group(expr), _) => expr |> of_expr |> _wrap(N.get_range(expr))
  | (AE.BinaryOp(_, lhs, rhs), _) =>
    _join(
      lhs |> of_expr |> _wrap(N.get_range(lhs)),
      rhs |> of_expr |> _wrap(N.get_range(rhs)),
    )
  | (AE.UnaryOp(_, expr), _) => expr |> of_expr |> _wrap(N.get_range(expr))
  | (AE.Closure(stmts), _) => stmts |> List.map(fst % of_stmt) |> of_list
  | (AE.DotAccess(expr, _), _) =>
    expr |> of_expr |> _wrap(N.get_range(expr))
  | (AE.FunctionCall(expr, args), _) =>
    _join(
      expr |> of_expr |> _wrap(N.get_range(expr)),
      args |> List.map(of_expr) |> of_list,
    )
  | (AE.Style(rules), _) =>
    rules |> List.map(fst % snd % of_expr) |> of_list

and of_jsx =
  fun
  | AE.Fragment(children) =>
    children
    |> List.map(child => child |> of_jsx_child |> _wrap(N.get_range(child)))
    |> of_list

  | AE.Tag((id, (_, range)), attrs, children)
  | AE.Component((id, (_, range)), attrs, children) =>
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
  | (AE.Node(tag), (_, range)) => tag |> of_jsx |> _wrap(range)
  | (AE.InlineExpression(expr), _) =>
    expr |> of_expr |> _wrap(N.get_range(expr))
  | (AE.Text(text), (_, range)) =>
    BinaryTree.create((range, Primitive(String(text))))

and of_jsx_attr =
  fun
  | AE.ID(id) => of_untyped_id(id)
  | AE.Class(id, None)
  | AE.Property(id, None) => of_untyped_id(id)
  | AE.Class(id, Some(expr))
  | AE.Property(id, Some(expr)) =>
    _join(of_untyped_id(id), expr |> of_expr |> _wrap(N.get_range(expr)))

and of_stmt =
  fun
  | AE.Variable(name, expr) =>
    _join(of_untyped_id(name), expr |> of_expr |> _wrap(N.get_range(expr)))
  | AE.Expression(expr) => expr |> of_expr |> _wrap(N.get_range(expr));

let of_args = args =>
  args
  |> _fold(
       fst
       % (
         (AE.{name, default, _}) => {
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
    |> List.map(((name, _)) => [of_untyped_id(name)])
    |> List.flatten
    |> of_list
  | A.Function(args, expr) =>
    _join(of_args(args), expr |> of_expr |> _wrap(N.get_range(expr)))
  | A.View(props, _, expr) =>
    _join(of_args(props), expr |> of_expr |> _wrap(N.get_range(expr)));

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
  | A.Import(_, imports) => imports |> _fold(fst % of_import)
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
          | Primitive(prim) => prim |> ~@KPrimitive.Plugin.pp
          },
          Range.create(start, end_) |> ~@Range.pp,
        ),
      ppf,
      tree,
    );
