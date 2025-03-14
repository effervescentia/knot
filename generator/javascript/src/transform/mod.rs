mod expression;
mod statement;

use crate::{
    ast::{JavaScript, Statement},
    knot, Options,
};
use kore::{internal, str};

impl JavaScript {
    pub fn from_program<Library>(
        path_to_root: &str,
        program: &knot::shape::Program,
        opts: &Options<Library>,
    ) -> Self
    where
        Library: internal::PlatformLibrary,
    {
        let module = &program.0;

        let statements = [
            Statement::import("@knot/runtime", vec![(str!("$knot"), None)], opts),
            Statement::from_module(path_to_root, module, opts),
            module
                .0
                .declarations
                .iter()
                .filter_map(|x| match (&x.0, x.0.is_public()) {
                    // type aliases are dropped in JavaScript
                    (knot::Declaration::TypeAlias { .. }, _) => None,

                    (_, true) => Some(Statement::export(x.0.binding(), opts)),

                    _ => None,
                })
                .collect(),
        ]
        .concat();

        Self(statements)
    }
}

#[cfg(test)]
mod tests {
    use crate::{ast::JavaScript, resolve::Resolver, test::MOCK_OPTIONS, Module, Options};
    use kore::str;
    use lang::ast;

    #[test]
    fn export_public_values() {
        let ast = JavaScript::from_program(
            ".",
            &ast::shape::Program(ast::shape::Module(ast::Module::new(
                vec![],
                vec![
                    ast::shape::Declaration(ast::Declaration::type_alias(
                        ast::Storage::public(str!("foo")),
                        ast::shape::TypeExpression(ast::TypeExpression::Primitive(
                            ast::TypePrimitive::Nil,
                        )),
                    )),
                    ast::shape::Declaration(ast::Declaration::constant(
                        ast::Storage::private(str!("bar")),
                        None,
                        ast::shape::Expression(ast::Expression::Primitive(ast::Primitive::Nil)),
                    )),
                    ast::shape::Declaration(ast::Declaration::constant(
                        ast::Storage::public(str!("fizz")),
                        None,
                        ast::shape::Expression(ast::Expression::Primitive(ast::Primitive::Nil)),
                    )),
                ],
            ))),
            &MOCK_OPTIONS,
        );

        insta::assert_debug_snapshot!(ast);
    }

    #[test]
    fn esm_export() {
        let ast = JavaScript::from_program(
            ".",
            &ast::shape::Program(ast::shape::Module(ast::Module::new(
                vec![],
                vec![ast::shape::Declaration(ast::Declaration::constant(
                    ast::Storage::public(str!("bar")),
                    None,
                    ast::shape::Expression(ast::Expression::Primitive(ast::Primitive::Nil)),
                ))],
            ))),
            &MOCK_OPTIONS,
        );

        insta::assert_debug_snapshot!(ast);
    }

    #[test]
    fn cjs_export() {
        let ast = JavaScript::from_program(
            ".",
            &ast::shape::Program(ast::shape::Module(ast::Module::new(
                vec![],
                vec![ast::shape::Declaration(ast::Declaration::constant(
                    ast::Storage::public(str!("bar")),
                    None,
                    ast::shape::Expression(ast::Expression::Primitive(ast::Primitive::Nil)),
                ))],
            ))),
            &Options {
                resolver: Resolver::new(Module::CJS),
                ..MOCK_OPTIONS
            },
        );

        insta::assert_debug_snapshot!(ast);
    }
}
