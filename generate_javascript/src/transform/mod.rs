mod expression;
mod statement;

use crate::{
    javascript::{JavaScript, Statement},
    Options,
};
use knot_language::ast;

impl JavaScript {
    pub fn from_module(value: &ast::ModuleShape, opts: &Options) -> Self {
        let statements = vec![
            Statement::import("@knot/runtime", vec![(String::from("$knot"), None)], opts),
            Statement::from_module(value, opts),
            value
                .0
                .declarations
                .iter()
                .filter_map(|x| match (&x.0, x.0.is_public()) {
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
    use knot_language::ast::{
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
                &ast::ModuleShape(ast::Module::new(
                    vec![],
                    vec![
                        ast::DeclarationShape(ast::Declaration::TypeAlias {
                            name: Storage(Visibility::Public, String::from("foo")),
                            value: ast::TypeExpressionShape(ast::TypeExpression::Nil)
                        }),
                        ast::DeclarationShape(ast::Declaration::Constant {
                            name: Storage(Visibility::Private, String::from("bar")),
                            value_type: None,
                            value: ast::ExpressionShape(ast::Expression::Primitive(
                                ast::Primitive::Nil
                            ))
                        }),
                        ast::DeclarationShape(ast::Declaration::Constant {
                            name: Storage(Visibility::Public, String::from("fizz")),
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
                Statement::Import(
                    String::from("@knot/runtime"),
                    vec![(String::from("$knot"), None)]
                ),
                Statement::Variable(String::from("bar"), Expression::Null),
                Statement::Variable(String::from("fizz"), Expression::Null),
                Statement::Export(String::from("fizz")),
            ])
        )
    }

    #[test]
    fn esm_export() {
        assert_eq!(
            JavaScript::from_module(
                &ast::ModuleShape(ast::Module::new(
                    vec![],
                    vec![ast::DeclarationShape(ast::Declaration::Constant {
                        name: Storage(Visibility::Public, String::from("bar")),
                        value_type: None,
                        value: ast::ExpressionShape(ast::Expression::Primitive(
                            ast::Primitive::Nil
                        ))
                    })]
                )),
                &OPTIONS
            ),
            JavaScript(vec![
                Statement::Import(
                    String::from("@knot/runtime"),
                    vec![(String::from("$knot"), None)]
                ),
                Statement::Variable(String::from("bar"), Expression::Null),
                Statement::Export(String::from("bar")),
            ])
        )
    }

    #[test]
    fn cjs_export() {
        assert_eq!(
            JavaScript::from_module(
                &ast::ModuleShape(ast::Module::new(
                    vec![],
                    vec![ast::DeclarationShape(ast::Declaration::Constant {
                        name: Storage(Visibility::Public, String::from("bar")),
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
                    String::from("$knot"),
                    Expression::DotAccess(
                        Box::new(Expression::FunctionCall(
                            Box::new(Expression::Identifier(String::from("require"))),
                            vec![Expression::String(String::from("@knot/runtime"))]
                        )),
                        String::from("$knot")
                    )
                ),
                Statement::Variable(String::from("bar"), Expression::Null),
                Statement::Assignment(
                    Expression::DotAccess(
                        Box::new(Expression::Identifier(String::from("exports")),),
                        String::from("bar")
                    ),
                    Expression::Identifier(String::from("bar"))
                ),
            ])
        )
    }
}
