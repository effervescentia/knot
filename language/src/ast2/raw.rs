use super::{
    capture::{self, Fragment},
    traversal::{NodeId, Visit, Visitor},
};

pub struct Node<Value> {
    pub value: Value,
    pub range: super::Range,
}

impl<Value> Node<Value> {
    pub const fn new(value: Value, range: super::Range) -> Self {
        Self { value, range }
    }
}

pub struct Expression(pub Node<super::Expression<Expression, Statement, Component>>);

pub struct Statement(pub Node<super::Statement<Expression>>);

pub struct Component(pub Node<super::Component<Expression, Component>>);

pub struct Parameter(pub Node<super::Parameter<Expression, TypeExpression>>);

pub struct Declaration(pub Node<super::Declaration<Expression, Parameter, Module, TypeExpression>>);

pub struct TypeExpression(pub Node<super::TypeExpression<TypeExpression>>);

pub struct Import(pub Node<super::Import>);

pub struct Module(pub Node<super::Module<Import, Declaration>>);

pub struct Program(pub Node<super::Program<Module>>);

impl capture::Capture<capture::Fragment> for Program {
    fn capture<F>(self, f: F, context: capture::Context) -> Vec<(NodeId, Fragment)>
    where
        F: Fn(capture::Fragment, capture::Context) -> (NodeId, capture::Context),
    {
        fn bind<Value, Bind, Capture>(
            bind: Bind,
            capture: Capture,
        ) -> impl Fn(Value, capture::Context) -> (NodeId, capture::Context)
        where
            Capture: Fn(capture::Fragment, capture::Context) -> (NodeId, capture::Context),
            Bind: Fn(Value) -> capture::Fragment,
        {
            move |x, c| capture(bind(x), c)
        }

        let visitor = Visitor {
            expression: &bind(capture::Fragment::Expression, &f),
            statement: &bind(capture::Fragment::Statement, &f),
            component: &bind(capture::Fragment::Component, &f),
            type_expression: &bind(capture::Fragment::TypeExpression, &f),
            parameter: &bind(capture::Fragment::Parameter, &f),
            declaration: &bind(capture::Fragment::Declaration, &f),
            import: &bind(capture::Fragment::Import, &f),
            module: &bind(capture::Fragment::Module, &f),
        };

        let super::Program(module) = self.0.value;
        let (_, context) = module.visit(&visitor, context);

        context.into_fragments()
    }
}

#[cfg(test)]
mod tests {
    use super::Node;
    use crate::ast2::{
        self,
        capture::{self, Capture},
        traversal::NodeId,
        Range,
    };
    use kore::{assert_eq, str};

    const fn mock_range() -> Range {
        Range::new((0, 0), (1, 1))
    }

    const fn mock_node<T>(value: T) -> Node<T> {
        Node::new(value, mock_range())
    }

    #[test]
    fn capture() {
        let context = capture::Context::default();
        let program = super::Program(mock_node(ast2::Program(super::Module(mock_node(
            ast2::Module {
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
                        value: super::TypeExpression(mock_node(ast2::TypeExpression::Nil)),
                    })),
                    super::Declaration(mock_node(ast2::Declaration::Constant {
                        storage: ast2::Storage::new(
                            ast2::Visibility::Public,
                            ast2::Binding::new(str!("MY_CONSTANT"), mock_range()),
                        ),
                        value_type: Some(super::TypeExpression(mock_node(
                            ast2::TypeExpression::Boolean,
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
                                    ast2::TypeExpression::Integer,
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
                                ast2::TypeExpression::String,
                            ))),
                            default_value: Some(super::Expression(mock_node(
                                ast2::Expression::Primitive(ast2::Primitive::String(str!(
                                    "my string"
                                ))),
                            ))),
                        }))],
                        body_type: Some(super::TypeExpression(mock_node(
                            ast2::TypeExpression::Nil,
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
                                ast2::TypeExpression::Float,
                            ))),
                            default_value: Some(super::Expression(mock_node(
                                ast2::Expression::Primitive(ast2::Primitive::Float(1.432, 4)),
                            ))),
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
                                        ast2::TypeExpression::Nil,
                                    )),
                                },
                            ))],
                        })),
                    })),
                ],
            },
        )))));

        assert_eq!(
            program.capture(|x, c| c.register(x), context),
            vec![
                (
                    NodeId(0),
                    capture::Fragment::Import(ast2::Import {
                        source: ast2::ImportSource::Local,
                        path: vec![str!("foo"), str!("bar"), str!("fizz")],
                        alias: None,
                    })
                ),
                (
                    NodeId(1),
                    capture::Fragment::TypeExpression(ast2::TypeExpression::Nil)
                ),
                (
                    NodeId(2),
                    capture::Fragment::Declaration(ast2::Declaration::TypeAlias {
                        storage: ast2::Storage::new(
                            ast2::Visibility::Public,
                            ast2::Binding::new(str!("MyType"), mock_range()),
                        ),
                        value: NodeId(1),
                    })
                ),
                (
                    NodeId(3),
                    capture::Fragment::TypeExpression(ast2::TypeExpression::Boolean)
                ),
                (
                    NodeId(4),
                    capture::Fragment::Expression(ast2::Expression::Primitive(
                        ast2::Primitive::Boolean(true),
                    ))
                ),
                (
                    NodeId(5),
                    capture::Fragment::Declaration(ast2::Declaration::Constant {
                        storage: ast2::Storage::new(
                            ast2::Visibility::Public,
                            ast2::Binding::new(str!("MY_CONSTANT"), mock_range()),
                        ),
                        value_type: Some(NodeId(3)),
                        value: NodeId(4),
                    })
                ),
                (
                    NodeId(6),
                    capture::Fragment::TypeExpression(ast2::TypeExpression::Integer)
                ),
                (
                    NodeId(7),
                    capture::Fragment::Declaration(ast2::Declaration::Enumerated {
                        storage: ast2::Storage::new(
                            ast2::Visibility::Public,
                            ast2::Binding::new(str!("MyEnum"), mock_range()),
                        ),
                        variants: vec![(str!("Empty"), vec![]), (str!("Number"), vec![NodeId(6)])],
                    })
                ),
                (
                    NodeId(8),
                    capture::Fragment::TypeExpression(ast2::TypeExpression::String)
                ),
                (
                    NodeId(9),
                    capture::Fragment::Expression(ast2::Expression::Primitive(
                        ast2::Primitive::String(str!("my string"))
                    ))
                ),
                (
                    NodeId(10),
                    capture::Fragment::Parameter(ast2::Parameter {
                        name: str!("zip"),
                        value_type: Some(NodeId(8)),
                        default_value: Some(NodeId(9)),
                    })
                ),
                (
                    NodeId(11),
                    capture::Fragment::TypeExpression(ast2::TypeExpression::Nil)
                ),
                (
                    NodeId(12),
                    capture::Fragment::Expression(ast2::Expression::Primitive(
                        ast2::Primitive::Nil,
                    ))
                ),
                (
                    NodeId(13),
                    capture::Fragment::Declaration(ast2::Declaration::Function {
                        storage: ast2::Storage::new(
                            ast2::Visibility::Public,
                            ast2::Binding::new(str!("my_function"), mock_range()),
                        ),
                        parameters: vec![NodeId(10)],
                        body_type: Some(NodeId(11)),
                        body: NodeId(12),
                    })
                ),
                (
                    NodeId(14),
                    capture::Fragment::TypeExpression(ast2::TypeExpression::Float)
                ),
                (
                    NodeId(15),
                    capture::Fragment::Expression(ast2::Expression::Primitive(
                        ast2::Primitive::Float(1.432, 4)
                    ))
                ),
                (
                    NodeId(16),
                    capture::Fragment::Parameter(ast2::Parameter {
                        name: str!("zap"),
                        value_type: Some(NodeId(14)),
                        default_value: Some(NodeId(15)),
                    })
                ),
                (
                    NodeId(17),
                    capture::Fragment::Expression(ast2::Expression::Primitive(
                        ast2::Primitive::Nil,
                    ))
                ),
                (
                    NodeId(18),
                    capture::Fragment::Declaration(ast2::Declaration::View {
                        storage: ast2::Storage::new(
                            ast2::Visibility::Public,
                            ast2::Binding::new(str!("MyView"), mock_range()),
                        ),
                        parameters: vec![NodeId(16)],
                        body: NodeId(17),
                    })
                ),
                (
                    NodeId(19),
                    capture::Fragment::Import(ast2::Import {
                        source: ast2::ImportSource::Local,
                        path: vec![str!("buzz")],
                        alias: Some(str!("Buzz")),
                    })
                ),
                (
                    NodeId(20),
                    capture::Fragment::TypeExpression(ast2::TypeExpression::Nil)
                ),
                (
                    NodeId(21),
                    capture::Fragment::Declaration(ast2::Declaration::TypeAlias {
                        storage: ast2::Storage::new(
                            ast2::Visibility::Public,
                            ast2::Binding::new(str!("NestedType"), mock_range()),
                        ),
                        value: NodeId(20),
                    })
                ),
                (
                    NodeId(22),
                    capture::Fragment::Module(ast2::Module {
                        imports: vec![NodeId(19)],
                        declarations: vec![NodeId(21)],
                    })
                ),
                (
                    NodeId(23),
                    capture::Fragment::Declaration(ast2::Declaration::Module {
                        storage: ast2::Storage::new(
                            ast2::Visibility::Public,
                            ast2::Binding::new(str!("my_module"), mock_range()),
                        ),
                        value: NodeId(22),
                    })
                ),
                (
                    NodeId(24),
                    capture::Fragment::Module(ast2::Module {
                        imports: vec![NodeId(0)],
                        declarations: vec![
                            NodeId(2),
                            NodeId(5),
                            NodeId(7),
                            NodeId(13),
                            NodeId(18),
                            NodeId(23),
                        ]
                    })
                )
            ]
        );
    }
}
