mod expression;
mod statement;

use crate::{
    javascript::{JavaScript, Statement},
    Options,
};
use kore::str;
use lang::ast;

impl JavaScript {
    pub fn from_module(path_to_root: &str, value: &ast::ModuleShape, opts: &Options) -> Self {
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

                    (_, true) => Some(Statement::export(x.0.name(), opts)),

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
    use lang::ast::{
        self,
        storage::{Storage, Visibility},
    };

    const OPTIONS: Options = Options {
        mode: Mode::Prod,
        module: Module::ESM,
    };

    #[test]
    fn export_public_values() {
        assert_eq!(
            JavaScript::from_module(
                ".",
                &ast::ModuleShape(ast::Module::new(
                    vec![],
                    vec![
                        ast::DeclarationShape(ast::Declaration::TypeAlias {
                            name: Storage(Visibility::Public, str!("foo")),
                            value: ast::TypeExpressionShape(ast::TypeExpression::Nil)
                        }),
                        ast::DeclarationShape(ast::Declaration::Constant {
                            name: Storage(Visibility::Private, str!("bar")),
                            value_type: None,
                            value: ast::ExpressionShape(ast::Expression::Primitive(
                                ast::Primitive::Nil
                            ))
                        }),
                        ast::DeclarationShape(ast::Declaration::Constant {
                            name: Storage(Visibility::Public, str!("fizz")),
                            value_type: None,
                            value: ast::ExpressionShape(ast::Expression::Primitive(
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
                &ast::ModuleShape(ast::Module::new(
                    vec![],
                    vec![ast::DeclarationShape(ast::Declaration::Constant {
                        name: Storage(Visibility::Public, str!("bar")),
                        value_type: None,
                        value: ast::ExpressionShape(ast::Expression::Primitive(
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
                &ast::ModuleShape(ast::Module::new(
                    vec![],
                    vec![ast::DeclarationShape(ast::Declaration::Constant {
                        name: Storage(Visibility::Public, str!("bar")),
                        value_type: None,
                        value: ast::ExpressionShape(ast::Expression::Primitive(
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
