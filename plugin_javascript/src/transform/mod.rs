mod expression;
mod statement;

use crate::{
    javascript::{JavaScript, Statement},
    Options,
};
use kore::str;
use lang::ast;

impl JavaScript {
    pub fn from_module(path_to_root: &str, value: &ast::shape::Module, opts: &Options) -> Self {
        let statements = [
            Statement::import("@knot/runtime", vec![(str!("$knot"), None)], opts),
            Statement::from_module(path_to_root, value, opts),
            value
                .0
                .declarations
                .iter()
                .filter_map(|x| match (&x.0, x.0.is_public()) {
                    // type aliases are dropped in JavaScript
                    (ast::Declaration::TypeAlias { .. }, _) => None,

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
    use crate::{
        javascript::{Expression, JavaScript, Statement},
        Mode, Module, Options,
    };
    use kore::str;
    use lang::ast;

    const OPTIONS: Options = Options {
        mode: Mode::Prod,
        module: Module::ESM,
    };

    #[test]
    fn export_public_values() {
        assert_eq!(
            JavaScript::from_module(
                ".",
                &ast::shape::Module(ast::Module::new(
                    vec![],
                    vec![
                        ast::shape::Declaration(ast::Declaration::TypeAlias {
                            storage: ast::Storage::public(str!("foo")),
                            value: ast::shape::TypeExpression(ast::TypeExpression::Primitive(
                                ast::TypePrimitive::Nil
                            ))
                        }),
                        ast::shape::Declaration(ast::Declaration::Constant {
                            storage: ast::Storage::private(str!("bar")),
                            value_type: None,
                            value: ast::shape::Expression(ast::Expression::Primitive(
                                ast::Primitive::Nil
                            ))
                        }),
                        ast::shape::Declaration(ast::Declaration::Constant {
                            storage: ast::Storage::public(str!("fizz")),
                            value_type: None,
                            value: ast::shape::Expression(ast::Expression::Primitive(
                                ast::Primitive::Nil
                            ))
                        }),
                    ]
                )),
                &OPTIONS
            ),
            JavaScript(vec![
                Statement::Import(str!("@knot/runtime"), vec![(str!("$knot"), None)]),
                Statement::Variable(str!("bar"), Expression::Null),
                Statement::Variable(str!("fizz"), Expression::Null),
                Statement::Export(str!("fizz")),
            ])
        );
    }

    #[test]
    fn esm_export() {
        assert_eq!(
            JavaScript::from_module(
                ".",
                &ast::shape::Module(ast::Module::new(
                    vec![],
                    vec![ast::shape::Declaration(ast::Declaration::Constant {
                        storage: ast::Storage::public(str!("bar")),
                        value_type: None,
                        value: ast::shape::Expression(ast::Expression::Primitive(
                            ast::Primitive::Nil
                        ))
                    })]
                )),
                &OPTIONS
            ),
            JavaScript(vec![
                Statement::Import(str!("@knot/runtime"), vec![(str!("$knot"), None)]),
                Statement::Variable(str!("bar"), Expression::Null),
                Statement::Export(str!("bar")),
            ])
        );
    }

    #[test]
    fn cjs_export() {
        assert_eq!(
            JavaScript::from_module(
                ".",
                &ast::shape::Module(ast::Module::new(
                    vec![],
                    vec![ast::shape::Declaration(ast::Declaration::Constant {
                        storage: ast::Storage::public(str!("bar")),
                        value_type: None,
                        value: ast::shape::Expression(ast::Expression::Primitive(
                            ast::Primitive::Nil
                        ))
                    })]
                )),
                &Options {
                    mode: Mode::Prod,
                    module: Module::CJS
                }
            ),
            JavaScript(vec![
                Statement::Variable(
                    str!("$knot"),
                    Expression::DotAccess(
                        Box::new(Expression::FunctionCall(
                            Box::new(Expression::Identifier(str!("require"))),
                            vec![Expression::String(str!("@knot/runtime"))]
                        )),
                        str!("$knot")
                    )
                ),
                Statement::Variable(str!("bar"), Expression::Null),
                Statement::Assignment(
                    Expression::DotAccess(
                        Box::new(Expression::Identifier(str!("exports")),),
                        str!("bar")
                    ),
                    Expression::Identifier(str!("bar"))
                ),
            ])
        );
    }
}
