use super::{analyze_module, expression, type_expression, Context};
use crate::parser::{
    declaration::{parameter::Parameter, Declaration, DeclarationNode},
    expression::ExpressionNode,
    position::Decrement,
    types::type_expression::TypeExpressionNode,
};
use combine::Stream;
use std::fmt::Debug;

pub fn analyze_declaration<T>(
    x: DeclarationNode<T, ()>,
    ctx: &mut Context,
) -> DeclarationNode<T, i32>
where
    T: Stream<Token = char>,
    T::Position: Copy + Debug + Decrement,
{
    let analyze_parameters = |xs: Vec<
        Parameter<ExpressionNode<T, ()>, TypeExpressionNode<T, ()>>,
    >,
                              ctx: &mut Context| {
        xs.into_iter()
            .map(
                |Parameter {
                     name,
                     value_type,
                     default_value,
                 }| Parameter {
                    name,
                    value_type: value_type
                        .map(|x| type_expression::analyze_type_expression(x, ctx)),
                    default_value: default_value.map(|x| expression::analyze_expression(x, ctx)),
                },
            )
            .collect::<Vec<_>>()
    };

    DeclarationNode(
        x.0.map(|x| match x {
            Declaration::TypeAlias { name, value } => Declaration::TypeAlias {
                name,
                value: type_expression::analyze_type_expression(value, ctx),
            },

            Declaration::Enumerated { name, variants } => Declaration::Enumerated {
                name,
                variants: variants
                    .into_iter()
                    .map(|(name, xs)| {
                        (
                            name,
                            xs.into_iter()
                                .map(|x| type_expression::analyze_type_expression(x, ctx))
                                .collect::<Vec<_>>(),
                        )
                    })
                    .collect::<Vec<_>>(),
            },

            Declaration::Constant {
                name,
                value_type,
                value,
            } => Declaration::Constant {
                name,
                value_type: value_type.map(|x| type_expression::analyze_type_expression(x, ctx)),
                value: expression::analyze_expression(value, ctx),
            },

            Declaration::Function {
                name,
                parameters,
                body_type,
                body,
            } => Declaration::Function {
                name,
                parameters: analyze_parameters(parameters, ctx),
                body_type: body_type.map(|x| type_expression::analyze_type_expression(x, ctx)),
                body: expression::analyze_expression(body, ctx),
            },

            Declaration::View {
                name,
                parameters,
                body,
            } => Declaration::View {
                name,
                parameters: analyze_parameters(parameters, ctx),
                body: expression::analyze_expression(body, ctx),
            },

            Declaration::Module { name, value } => Declaration::Module {
                name,
                value: analyze_module(value, ctx),
            },
        })
        .with_context(ctx.generate_id()),
    )
}

#[cfg(test)]
mod tests {
    use super::analyze_declaration;
    use crate::{
        analyzer::Context,
        parser::{
            declaration::{
                parameter::Parameter,
                storage::{Storage, Visibility},
                Declaration, DeclarationNode,
            },
            expression::{primitive::Primitive, Expression, ExpressionNode},
            module::{
                import::{Import, Source, Target},
                Module, ModuleNode,
            },
            node::Node,
            range::Range,
            types::type_expression::{TypeExpression, TypeExpressionNode},
            CharStream,
        },
    };

    const RANGE: Range<CharStream> = Range::chars((1, 1), (1, 1));

    #[test]
    fn type_alias() {
        let ctx = &mut Context::new();

        let result = analyze_declaration(
            DeclarationNode(Node(
                Declaration::TypeAlias {
                    name: Storage(Visibility::Public, String::from("Foo")),
                    value: TypeExpressionNode(Node(TypeExpression::Nil, RANGE, ())),
                },
                RANGE,
                (),
            )),
            ctx,
        );

        assert_eq!(
            result,
            DeclarationNode(Node(
                Declaration::TypeAlias {
                    name: Storage(Visibility::Public, String::from("Foo")),
                    value: TypeExpressionNode(Node(TypeExpression::Nil, RANGE, 0)),
                },
                RANGE,
                1,
            ))
        )
    }

    #[test]
    fn enumerated() {
        let ctx = &mut Context::new();

        let result = analyze_declaration(
            DeclarationNode(Node(
                Declaration::Enumerated {
                    name: Storage(Visibility::Public, String::from("Foo")),
                    variants: vec![(
                        String::from("Bar"),
                        vec![TypeExpressionNode(Node(TypeExpression::Nil, RANGE, ()))],
                    )],
                },
                RANGE,
                (),
            )),
            ctx,
        );

        assert_eq!(
            result,
            DeclarationNode(Node(
                Declaration::Enumerated {
                    name: Storage(Visibility::Public, String::from("Foo")),
                    variants: vec![(
                        String::from("Bar"),
                        vec![TypeExpressionNode(Node(TypeExpression::Nil, RANGE, 0))],
                    )],
                },
                RANGE,
                1,
            ))
        )
    }

    #[test]
    fn constant() {
        let ctx = &mut Context::new();

        let result = analyze_declaration(
            DeclarationNode(Node(
                Declaration::Constant {
                    name: Storage(Visibility::Public, String::from("FOO")),
                    value_type: Some(TypeExpressionNode(Node(TypeExpression::Nil, RANGE, ()))),
                    value: ExpressionNode(Node(Expression::Primitive(Primitive::Nil), RANGE, ())),
                },
                RANGE,
                (),
            )),
            ctx,
        );

        assert_eq!(
            result,
            DeclarationNode(Node(
                Declaration::Constant {
                    name: Storage(Visibility::Public, String::from("FOO")),
                    value_type: Some(TypeExpressionNode(Node(TypeExpression::Nil, RANGE, 0))),
                    value: ExpressionNode(Node(Expression::Primitive(Primitive::Nil), RANGE, 1)),
                },
                RANGE,
                2,
            ))
        )
    }

    #[test]
    fn function() {
        let ctx = &mut Context::new();

        let result = analyze_declaration(
            DeclarationNode(Node(
                Declaration::Function {
                    name: Storage(Visibility::Public, String::from("Foo")),
                    parameters: vec![Parameter {
                        name: String::from("bar"),
                        value_type: Some(TypeExpressionNode(Node(TypeExpression::Nil, RANGE, ()))),
                        default_value: Some(ExpressionNode(Node(
                            Expression::Primitive(Primitive::Nil),
                            RANGE,
                            (),
                        ))),
                    }],
                    body_type: Some(TypeExpressionNode(Node(TypeExpression::Nil, RANGE, ()))),
                    body: ExpressionNode(Node(Expression::Primitive(Primitive::Nil), RANGE, ())),
                },
                RANGE,
                (),
            )),
            ctx,
        );

        assert_eq!(
            result,
            DeclarationNode(Node(
                Declaration::Function {
                    name: Storage(Visibility::Public, String::from("Foo")),
                    parameters: vec![Parameter {
                        name: String::from("bar"),
                        value_type: Some(TypeExpressionNode(Node(TypeExpression::Nil, RANGE, 0))),
                        default_value: Some(ExpressionNode(Node(
                            Expression::Primitive(Primitive::Nil),
                            RANGE,
                            1
                        ))),
                    }],
                    body_type: Some(TypeExpressionNode(Node(TypeExpression::Nil, RANGE, 2))),
                    body: ExpressionNode(Node(Expression::Primitive(Primitive::Nil), RANGE, 3)),
                },
                RANGE,
                4,
            ))
        )
    }

    #[test]
    fn view() {
        let ctx = &mut Context::new();

        let result = analyze_declaration(
            DeclarationNode(Node(
                Declaration::View {
                    name: Storage(Visibility::Public, String::from("Foo")),
                    parameters: vec![Parameter {
                        name: String::from("bar"),
                        value_type: Some(TypeExpressionNode(Node(TypeExpression::Nil, RANGE, ()))),
                        default_value: Some(ExpressionNode(Node(
                            Expression::Primitive(Primitive::Nil),
                            RANGE,
                            (),
                        ))),
                    }],
                    body: ExpressionNode(Node(Expression::Primitive(Primitive::Nil), RANGE, ())),
                },
                RANGE,
                (),
            )),
            ctx,
        );

        assert_eq!(
            result,
            DeclarationNode(Node(
                Declaration::View {
                    name: Storage(Visibility::Public, String::from("Foo")),
                    parameters: vec![Parameter {
                        name: String::from("bar"),
                        value_type: Some(TypeExpressionNode(Node(TypeExpression::Nil, RANGE, 0))),
                        default_value: Some(ExpressionNode(Node(
                            Expression::Primitive(Primitive::Nil),
                            RANGE,
                            1,
                        ))),
                    }],
                    body: ExpressionNode(Node(Expression::Primitive(Primitive::Nil), RANGE, 2)),
                },
                RANGE,
                3,
            ))
        )
    }

    #[test]
    fn module() {
        let ctx = &mut Context::new();

        let result = analyze_declaration(
            DeclarationNode(Node(
                Declaration::Module {
                    name: Storage(Visibility::Public, String::from("Foo")),
                    value: ModuleNode(
                        Module {
                            imports: vec![Import {
                                source: Source::Root,
                                path: vec![String::from("bar"), String::from("fizz")],
                                aliases: Some(vec![(Target::Module, Some(String::from("Fizz")))]),
                            }],
                            declarations: vec![DeclarationNode(Node(
                                Declaration::Constant {
                                    name: Storage(Visibility::Public, String::from("BUZZ")),
                                    value_type: Some(TypeExpressionNode(Node(
                                        TypeExpression::Nil,
                                        RANGE,
                                        (),
                                    ))),
                                    value: ExpressionNode(Node(
                                        Expression::Primitive(Primitive::Nil),
                                        RANGE,
                                        (),
                                    )),
                                },
                                RANGE,
                                (),
                            ))],
                        },
                        (),
                    ),
                },
                RANGE,
                (),
            )),
            ctx,
        );

        assert_eq!(
            result,
            DeclarationNode(Node(
                Declaration::Module {
                    name: Storage(Visibility::Public, String::from("Foo")),
                    value: ModuleNode(
                        Module {
                            imports: vec![Import {
                                source: Source::Root,
                                path: vec![String::from("bar"), String::from("fizz")],
                                aliases: Some(vec![(Target::Module, Some(String::from("Fizz")))]),
                            }],
                            declarations: vec![DeclarationNode(Node(
                                Declaration::Constant {
                                    name: Storage(Visibility::Public, String::from("BUZZ")),
                                    value_type: Some(TypeExpressionNode(Node(
                                        TypeExpression::Nil,
                                        RANGE,
                                        0,
                                    ))),
                                    value: ExpressionNode(Node(
                                        Expression::Primitive(Primitive::Nil),
                                        RANGE,
                                        1,
                                    )),
                                },
                                RANGE,
                                2,
                            ))],
                        },
                        3,
                    ),
                },
                RANGE,
                4,
            ))
        )
    }
}
