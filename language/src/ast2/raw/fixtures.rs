use crate::ast2::{self, Range};
use kore::str;

pub const fn mock_range() -> Range {
    Range::new((0, 0), (1, 1))
}

pub fn raw_program() -> super::Program {
    const fn mock_node<V>(value: V) -> super::Node<V> {
        super::Node::new(value, mock_range())
    }

    super::Program(super::Module(mock_node(ast2::Module {
        imports: vec![super::Import(mock_node(ast2::Import {
            source: ast2::ImportSource::Local,
            path: vec![str!("foo"), str!("bar"), str!("fizz")],
            alias: None,
        }))],
        declarations: vec![
            super::Declaration(mock_node(ast2::Declaration::TypeAlias {
                storage: ast2::Storage::new(
                    ast2::Visibility::Public,
                    ast2::Binding::new(str!("MyType"), mock_range()),
                ),
                value: super::TypeExpression(mock_node(ast2::TypeExpression::Primitive(
                    ast2::TypePrimitive::Nil,
                ))),
            })),
            super::Declaration(mock_node(ast2::Declaration::Constant {
                storage: ast2::Storage::new(
                    ast2::Visibility::Public,
                    ast2::Binding::new(str!("MY_CONSTANT"), mock_range()),
                ),
                value_type: Some(super::TypeExpression(mock_node(
                    ast2::TypeExpression::Primitive(ast2::TypePrimitive::Boolean),
                ))),
                value: super::Expression(mock_node(ast2::Expression::Primitive(
                    ast2::Primitive::Boolean(true),
                ))),
            })),
            super::Declaration(mock_node(ast2::Declaration::Enumerated {
                storage: ast2::Storage::new(
                    ast2::Visibility::Public,
                    ast2::Binding::new(str!("MyEnum"), mock_range()),
                ),
                variants: vec![
                    (str!("Empty"), vec![]),
                    (
                        str!("Number"),
                        vec![super::TypeExpression(mock_node(
                            ast2::TypeExpression::Primitive(ast2::TypePrimitive::Integer),
                        ))],
                    ),
                ],
            })),
            super::Declaration(mock_node(ast2::Declaration::Function {
                storage: ast2::Storage::new(
                    ast2::Visibility::Public,
                    ast2::Binding::new(str!("my_function"), mock_range()),
                ),
                parameters: vec![super::Parameter(mock_node(ast2::Parameter {
                    name: str!("zip"),
                    value_type: Some(super::TypeExpression(mock_node(
                        ast2::TypeExpression::Primitive(ast2::TypePrimitive::String),
                    ))),
                    default_value: Some(super::Expression(mock_node(ast2::Expression::Primitive(
                        ast2::Primitive::String(str!("my string")),
                    )))),
                }))],
                body_type: Some(super::TypeExpression(mock_node(
                    ast2::TypeExpression::Primitive(ast2::TypePrimitive::Nil),
                ))),
                body: super::Expression(mock_node(ast2::Expression::Primitive(
                    ast2::Primitive::Nil,
                ))),
            })),
            super::Declaration(mock_node(ast2::Declaration::View {
                storage: ast2::Storage::new(
                    ast2::Visibility::Public,
                    ast2::Binding::new(str!("MyView"), mock_range()),
                ),
                parameters: vec![super::Parameter(mock_node(ast2::Parameter {
                    name: str!("zap"),
                    value_type: Some(super::TypeExpression(mock_node(
                        ast2::TypeExpression::Primitive(ast2::TypePrimitive::Float),
                    ))),
                    default_value: Some(super::Expression(mock_node(ast2::Expression::Primitive(
                        ast2::Primitive::Float(1.432, 4),
                    )))),
                }))],
                body: super::Expression(mock_node(ast2::Expression::Primitive(
                    ast2::Primitive::Nil,
                ))),
            })),
            super::Declaration(mock_node(ast2::Declaration::Module {
                storage: ast2::Storage::new(
                    ast2::Visibility::Public,
                    ast2::Binding::new(str!("my_module"), mock_range()),
                ),
                value: super::Module(mock_node(ast2::Module {
                    imports: vec![super::Import(mock_node(ast2::Import {
                        source: ast2::ImportSource::Local,
                        path: vec![str!("buzz")],
                        alias: Some(str!("Buzz")),
                    }))],
                    declarations: vec![super::Declaration(mock_node(
                        ast2::Declaration::TypeAlias {
                            storage: ast2::Storage::new(
                                ast2::Visibility::Public,
                                ast2::Binding::new(str!("NestedType"), mock_range()),
                            ),
                            value: super::TypeExpression(mock_node(
                                ast2::TypeExpression::Primitive(ast2::TypePrimitive::Nil),
                            )),
                        },
                    ))],
                })),
            })),
        ],
    })))
}
