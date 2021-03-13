/**
 Utitlities for converting module AST into JavaScript code.
 */
open Kore;
open AST;

let _print_many = (printer, print) => {
  let rec loop =
    fun
    | [] => ()
    | [x] => x |> print(printer)
    | [x, ...xs] => {
        x |> print(printer);
        printer(", ");
        loop(xs);
      };
  loop;
};

let number =
  fun
  | Integer(value) => value |> Int64.to_string
  | Float(value, precision) => value |> Print.fmt("%.*g", precision);

let string = String.escaped % Print.fmt("\"%s\"");

let common_import =
    ({print, resolve}: output_t, name: namespace_t, main: string) =>
  name
  |> resolve
  |> string
  |> Print.fmt("var %s = require(%s);\n", main)
  |> print;

let common_export = (print: print_t, name: string) =>
  Print.fmt("exports.%s = %s;\n", name, name) |> print;

let es6_import =
    ({print, resolve}: output_t, name: namespace_t, main: string) =>
  name
  |> resolve
  |> string
  |> Print.fmt("import %s from %s;\n", main)
  |> print;

let es6_export = (print: print_t, name: string) =>
  Print.fmt("export { %s };\n", name) |> print;

let primitive = (print: print_t) =>
  fun
  | Boolean(value) => (value ? "true" : "false") |> print
  | Number(value) => value |> Block.value |> number |> print
  | String(value) => value |> string |> print
  | Nil => print("null");

let rec expression = (print: print_t) =>
  fun
  | Primitive(value) => value |> Block.value |> primitive(print)
  | Identifier(value) => value |> Block.value |> print
  | Group(value) => {
      print("(");
      value |> Block.value |> expression(print);
      print(")");
    }
  | Closure(value) =>
    value
    |> Block.value
    |> List.excl(EmptyStatement)
    |> (
      values =>
        List.is_empty(values)
          ? print("null")
          : {
            print("(function(){\n");

            let rec loop =
              fun
              | [] => ()
              | [x] => x |> statement(~is_last=true, print)

              | [x, ...xs] => {
                  x |> statement(print);
                  loop(xs);
                };
            loop(values);

            print("})()");
          }
    )
  | UnaryOp(op, value) => value |> unary_op(print, op)
  | BinaryOp(op, lhs, rhs) => binary_op(print, op, lhs, rhs)
  | JSX(value) => value |> jsx(print)
and statement = (~is_last=false, print: print_t) =>
  fun
  | Variable(name, value) => {
      Print.fmt("var %s = ", name) |> print;
      value |> expression(print);
      print(";\n");

      if (is_last) {
        print("return null;\n");
      };
    }
  | Expression(value) => {
      if (is_last) {
        print("return ");
      };

      value |> expression(print);
      print(";\n");
    }
  | EmptyStatement => ()
and unary_op = (print: print_t) => {
  let print' = (symbol, value) => {
    symbol |> print;
    value |> expression(print);
  };

  fun
  | Negative => print'("-")
  | Positive => print'("+")
  | Not => print'("!");
}
and binary_op = (print: print_t) => {
  let print' = (symbol, lhs, rhs) => {
    print("(");
    lhs |> expression(print);
    symbol |> Print.fmt(" %s ") |> print;
    rhs |> expression(print);
    print(")");
  };

  fun
  | LogicalAnd => print'("&&")
  | LogicalOr => print'("||")
  | LessOrEqual => print'("<=")
  | LessThan => print'("<")
  | GreaterOrEqual => print'(">=")
  | GreaterThan => print'(">")
  | Equal => print'("===")
  | Unequal => print'("!==")
  | Add => print'("+")
  | Subtract => print'("-")
  | Multiply => print'("*")
  | Divide => print'("/")
  | Exponent => (
      (lhs, rhs) => {
        print("Math.pow(");
        lhs |> expression(print);
        print(", ");
        rhs |> expression(print);
        print(")");
      }
    );
}
and jsx = (print: print_t) =>
  fun
  | Tag(name, attrs, values) => {
      name |> string |> Print.fmt("$knot.jsx.createTag(%s, ") |> print;

      attrs |> jsx_attrs(print);

      values
      |> List.iter(x => {
           print(", ");
           x |> jsx_child(print);
         });

      print(")");
    }
  | Fragment(values) => {
      print("$knot.jsx.createFragment(");

      _print_many(print, jsx_child, values);

      print(")");
    }
and jsx_child = (print: print_t) =>
  fun
  | Node(value) => value |> jsx(print)
  | Text(value) => value |> string |> print
  | InlineExpression(value) => value |> expression(print)
and jsx_attrs = (print: print_t, attrs: list(jsx_attribute_t)) =>
  if (List.is_empty(attrs)) {
    print("{}");
  } else {
    /* assumes that ID and unique class names / prop names only appear once at most  */
    let (classes, props) =
      attrs
      |> List.fold_left(
           ((c, p)) =>
             fun
             | Property(name, expr) => (
                 c,
                 [
                   (
                     name,
                     (
                       () =>
                         expression(
                           print,
                           expr
                           |?: (name |> Block.create(Cursor.zero) |> of_id),
                         )
                     ),
                   ),
                   ...p,
                 ],
               )
             | Class(name, None) => (
                 [
                   (
                     name,
                     (() => name |> Print.fmt(".%s") |> string |> print),
                   ),
                   ...c,
                 ],
                 p,
               )
             | Class(name, Some(expr)) => (
                 [
                   (
                     name,
                     (
                       () => {
                         print("(");
                         expr |> expression(print);
                         name
                         |> Print.fmt(".%s")
                         |> string
                         |> Print.fmt(" ? %s : \"\")")
                         |> print;
                       }
                     ),
                   ),
                   ...c,
                 ],
                 p,
               )
             | ID(name) => (
                 c,
                 [("id", (() => name |> string |> print)), ...p],
               ),
           ([], []),
         );

    let props =
      List.is_empty(classes)
        ? props
        : [
          (
            "className",
            () => {
              let rec loop =
                fun
                | [] => print("\"\"")
                | [(name, print_classname), ...xs] => {
                    print_classname();

                    if (!List.is_empty(xs)) {
                      print(" + ");
                      loop(xs);
                    };
                  };
              loop(classes);
            },
          ),
          ...props,
        ];

    print("{ ");
    _print_many(
      print,
      (_, (name, print_value)) => {
        Print.fmt("%s: ", name) |> print;
        print_value();
      },
      props,
    );
    print(" }");
  };

let constant = (print: print_t, name: string, value: expression_t) => {
  Print.fmt("var %s = ", name) |> print;
  value |> expression(print);
  print(";\n");
};

let declaration =
    (
      print: print_t,
      module_type: Target.module_t,
      name: string,
      decl: declaration_t,
    ) => {
  switch (decl) {
  | Constant(value) => constant(print, name, value)
  };

  switch (module_type) {
  | Common => common_export(print, name)
  | ES6 => es6_export(print, name)
  };
};

let generate =
    (
      module_type: Target.module_t,
      {print, resolve} as output: output_t,
      ast: program_t,
    ) => {
  let has_no_declarations =
    ast
    |> List.filter(
         fun
         | Declaration(_) => true
         | _ => false,
       )
    |> List.is_empty;

  if (!has_no_declarations) {
    "@knot/runtime"
    |> string
    |> (
      switch (module_type) {
      | Common => Print.fmt("var $knot = require(%s);\n")
      | ES6 => Print.fmt("import $knot from %s;\n")
      }
    )
    |> print;
  };

  let resolve =
    resolve
    % (
      fun
      | "" => "."
      | relative =>
        String.starts_with("..", relative) ? relative : "./" ++ relative
    );

  ast
  |> List.iter(
       fun
       | Import(name, main) =>
         switch (module_type) {
         | Common => common_import({...output, resolve}, name, main)
         | ES6 => es6_import({...output, resolve}, name, main)
         }
       | Declaration(name, decl) =>
         declaration(print, module_type, name, decl)
       | EmptyModuleStatement => (),
     );

  if (has_no_declarations) {
    switch (module_type) {
    | Common => print("module.exports = {};\n")
    | ES6 => print("export {};\n")
    };
  };
};
