use crate::ast2::{
    self,
    typed::{self, TypeRef},
    walk::{NodeId, Visit, Walk},
    Range,
};
use kore::invariant;
use std::collections::HashMap;

struct Visitor<'a> {
    next_id: usize,
    types: HashMap<NodeId, typed::TypeRef<'a>>,
}

impl<'a> Visitor<'a> {
    const fn new(types: HashMap<NodeId, typed::TypeRef<'a>>) -> Self {
        Self { next_id: 0, types }
    }

    fn next(&mut self) -> typed::TypeRef<'a> {
        let id = NodeId(self.next_id);
        self.next_id += 1;

        self.types
            .remove(&id)
            .unwrap_or_else(|| invariant!("type does not exist by id {id:?}"))
    }

    fn wrap<Value, Result, F>(mut self, value: Value, range: Range, f: F) -> (Result, Self)
    where
        F: Fn(typed::Node<Value, typed::TypeRef<'a>>) -> Result,
    {
        (f(typed::Node::new(value, self.next(), range)), self)
    }
}

impl<'a> Visit for Visitor<'a> {
    type Expression = typed::Expression<TypeRef<'a>>;
    type Statement = typed::Statement<TypeRef<'a>>;
    type Component = typed::Component<TypeRef<'a>>;
    type TypeExpression = typed::TypeExpression<TypeRef<'a>>;
    type Parameter = typed::Parameter<TypeRef<'a>>;
    type Declaration = typed::Declaration<TypeRef<'a>>;
    type Import = typed::Import<TypeRef<'a>>;
    type Module = typed::Module<TypeRef<'a>>;

    fn expression(
        self,
        x: ast2::Expression<Self::Expression, Self::Statement, Self::Component>,
        r: Range,
    ) -> (Self::Expression, Self) {
        self.wrap(x, r, typed::Expression)
    }

    fn statement(self, x: ast2::Statement<Self::Expression>, r: Range) -> (Self::Statement, Self) {
        self.wrap(x, r, typed::Statement)
    }

    fn component(
        self,
        x: ast2::Component<Self::Expression, Self::Component>,
        r: Range,
    ) -> (Self::Component, Self) {
        self.wrap(x, r, typed::Component)
    }

    fn type_expression(
        self,
        x: ast2::TypeExpression<Self::TypeExpression>,
        r: Range,
    ) -> (Self::TypeExpression, Self) {
        self.wrap(x, r, typed::TypeExpression)
    }

    fn parameter(
        self,
        x: ast2::Parameter<Self::Expression, Self::TypeExpression>,
        r: Range,
    ) -> (Self::Parameter, Self) {
        self.wrap(x, r, typed::Parameter)
    }

    fn declaration(
        self,
        x: ast2::Declaration<Self::Expression, Self::Parameter, Self::Module, Self::TypeExpression>,
        r: Range,
    ) -> (Self::Declaration, Self) {
        self.wrap(x, r, typed::Declaration)
    }

    fn import(self, x: ast2::Import, r: Range) -> (Self::Import, Self) {
        self.wrap(x, r, typed::Import)
    }

    fn module(
        self,
        x: ast2::Module<Self::Import, Self::Declaration>,
        r: Range,
    ) -> (Self::Module, Self) {
        self.wrap(x, r, typed::Module)
    }
}

#[allow(clippy::multiple_inherent_impl)]
impl super::Program {
    pub fn augment(self, types: HashMap<NodeId, typed::TypeRef>) -> typed::Program<typed::TypeRef> {
        let (result, _) = self.0.walk(Visitor::new(types));

        typed::Program(result)
    }
}

#[cfg(test)]
mod tests {
    use crate::ast2::{
        self,
        raw::fixtures,
        typed::{self, Kind, TypeRef, TypeShape},
        walk::NodeId,
    };
    use kore::{assert_eq, str};
    use std::collections::HashMap;

    const fn mock_node<V, T>(value: V, type_: T) -> typed::Node<V, T> {
        typed::Node::new(value, type_, fixtures::mock_range())
    }

    #[test]
    fn augment() {
        let program = fixtures::raw_program();
        let enum_type = TypeRef(TypeShape::Enumerated(vec![
            (str!("Empty"), vec![]),
            (str!("Number"), vec![&TypeRef(TypeShape::Integer)]),
        ]));
        let function_type = TypeRef(TypeShape::Function(
            vec![&TypeRef(TypeShape::String)],
            Box::new(&TypeRef(TypeShape::Nil)),
        ));
        let view_type = TypeRef(TypeShape::View(vec![&TypeRef(TypeShape::Float)]));
        let module_type = TypeRef(TypeShape::Module(vec![(
            str!("NestedType"),
            Kind::Type,
            &TypeRef(TypeShape::Nil),
        )]));
        let types = HashMap::from_iter(vec![
            (
                NodeId(0),
                TypeRef(TypeShape::Module(vec![(
                    str!("bar"),
                    Kind::Value,
                    &TypeRef(TypeShape::Boolean),
                )])),
            ),
            (NodeId(1), TypeRef(TypeShape::Nil)),
            (NodeId(2), TypeRef(TypeShape::Nil)),
            (NodeId(3), TypeRef(TypeShape::Boolean)),
            (NodeId(4), TypeRef(TypeShape::Boolean)),
            (NodeId(5), TypeRef(TypeShape::Boolean)),
            (NodeId(6), TypeRef(TypeShape::Integer)),
            (
                NodeId(7),
                TypeRef(TypeShape::Enumerated(vec![
                    (str!("Empty"), vec![]),
                    (str!("Number"), vec![&TypeRef(TypeShape::Integer)]),
                ])),
            ),
            (NodeId(8), TypeRef(TypeShape::String)),
            (NodeId(9), TypeRef(TypeShape::String)),
            (NodeId(10), TypeRef(TypeShape::String)),
            (NodeId(11), TypeRef(TypeShape::Nil)),
            (NodeId(12), TypeRef(TypeShape::Nil)),
            (
                NodeId(13),
                TypeRef(TypeShape::Function(
                    vec![&TypeRef(TypeShape::String)],
                    Box::new(&TypeRef(TypeShape::Nil)),
                )),
            ),
            (NodeId(14), TypeRef(TypeShape::Float)),
            (NodeId(15), TypeRef(TypeShape::Float)),
            (NodeId(16), TypeRef(TypeShape::Float)),
            (NodeId(17), TypeRef(TypeShape::Nil)),
            (
                NodeId(18),
                TypeRef(TypeShape::View(vec![&TypeRef(TypeShape::Float)])),
            ),
            (
                NodeId(19),
                TypeRef(TypeShape::Module(vec![(
                    str!("zip"),
                    Kind::Type,
                    &TypeRef(TypeShape::String),
                )])),
            ),
            (NodeId(20), TypeRef(TypeShape::Nil)),
            (NodeId(21), TypeRef(TypeShape::Nil)),
            (
                NodeId(22),
                TypeRef(TypeShape::Module(vec![(
                    str!("NestedType"),
                    Kind::Type,
                    &TypeRef(TypeShape::Nil),
                )])),
            ),
            (
                NodeId(23),
                TypeRef(TypeShape::Module(vec![(
                    str!("NestedType"),
                    Kind::Type,
                    &TypeRef(TypeShape::Nil),
                )])),
            ),
            (
                NodeId(24),
                TypeRef(TypeShape::Module(vec![
                    (str!("MyType"), Kind::Type, &TypeRef(TypeShape::Nil)),
                    (
                        str!("MY_CONSTANT"),
                        Kind::Value,
                        &TypeRef(TypeShape::Boolean),
                    ),
                    (str!("MyEnum"), Kind::Value, &enum_type),
                    (str!("my_function"), Kind::Value, &function_type),
                    (str!("MyView"), Kind::Value, &view_type),
                    (str!("my_module"), Kind::Mixed, &module_type),
                ])),
            ),
        ]);

        assert_eq!(
            program.augment(types),
            typed::Program(typed::Module(mock_node(
                ast2::Module {
                    imports: vec![typed::Import(mock_node(
                        ast2::Import {
                            source: ast2::ImportSource::Local,
                            path: vec![str!("foo"), str!("bar"), str!("fizz")],
                            alias: None,
                        },
                        TypeRef(TypeShape::Module(vec![(
                            str!("bar"),
                            Kind::Value,
                            &TypeRef(TypeShape::Boolean)
                        )]))
                    ))],
                    declarations: vec![
                        typed::Declaration(mock_node(
                            ast2::Declaration::TypeAlias {
                                storage: ast2::Storage::new(
                                    ast2::Visibility::Public,
                                    ast2::Binding::new(str!("MyType"), fixtures::mock_range()),
                                ),
                                value: typed::TypeExpression(mock_node(
                                    ast2::TypeExpression::Primitive(ast2::TypePrimitive::Nil),
                                    TypeRef(TypeShape::Nil)
                                )),
                            },
                            TypeRef(TypeShape::Nil)
                        )),
                        typed::Declaration(mock_node(
                            ast2::Declaration::Constant {
                                storage: ast2::Storage::new(
                                    ast2::Visibility::Public,
                                    ast2::Binding::new(str!("MY_CONSTANT"), fixtures::mock_range()),
                                ),
                                value_type: Some(typed::TypeExpression(mock_node(
                                    ast2::TypeExpression::Primitive(ast2::TypePrimitive::Boolean),
                                    TypeRef(TypeShape::Boolean)
                                ))),
                                value: typed::Expression(mock_node(
                                    ast2::Expression::Primitive(ast2::Primitive::Boolean(true)),
                                    TypeRef(TypeShape::Boolean)
                                )),
                            },
                            TypeRef(TypeShape::Boolean)
                        )),
                        typed::Declaration(mock_node(
                            ast2::Declaration::Enumerated {
                                storage: ast2::Storage::new(
                                    ast2::Visibility::Public,
                                    ast2::Binding::new(str!("MyEnum"), fixtures::mock_range()),
                                ),
                                variants: vec![
                                    (str!("Empty"), vec![]),
                                    (
                                        str!("Number"),
                                        vec![typed::TypeExpression(mock_node(
                                            ast2::TypeExpression::Primitive(
                                                ast2::TypePrimitive::Integer
                                            ),
                                            TypeRef(TypeShape::Integer)
                                        ))],
                                    ),
                                ],
                            },
                            TypeRef(TypeShape::Enumerated(vec![
                                (str!("Empty"), vec![]),
                                (str!("Number"), vec![&TypeRef(TypeShape::Integer)]),
                            ]))
                        )),
                        typed::Declaration(mock_node(
                            ast2::Declaration::Function {
                                storage: ast2::Storage::new(
                                    ast2::Visibility::Public,
                                    ast2::Binding::new(str!("my_function"), fixtures::mock_range()),
                                ),
                                parameters: vec![typed::Parameter(mock_node(
                                    ast2::Parameter {
                                        name: str!("zip"),
                                        value_type: Some(typed::TypeExpression(mock_node(
                                            ast2::TypeExpression::Primitive(
                                                ast2::TypePrimitive::String
                                            ),
                                            TypeRef(TypeShape::String)
                                        ))),
                                        default_value: Some(typed::Expression(mock_node(
                                            ast2::Expression::Primitive(ast2::Primitive::String(
                                                str!("my string")
                                            )),
                                            TypeRef(TypeShape::String)
                                        ))),
                                    },
                                    TypeRef(TypeShape::String)
                                ))],
                                body_type: Some(typed::TypeExpression(mock_node(
                                    ast2::TypeExpression::Primitive(ast2::TypePrimitive::Nil),
                                    TypeRef(TypeShape::Nil)
                                ))),
                                body: typed::Expression(mock_node(
                                    ast2::Expression::Primitive(ast2::Primitive::Nil),
                                    TypeRef(TypeShape::Nil)
                                )),
                            },
                            TypeRef(TypeShape::Function(
                                vec![&TypeRef(TypeShape::String)],
                                Box::new(&TypeRef(TypeShape::Nil))
                            ))
                        )),
                        typed::Declaration(mock_node(
                            ast2::Declaration::View {
                                storage: ast2::Storage::new(
                                    ast2::Visibility::Public,
                                    ast2::Binding::new(str!("MyView"), fixtures::mock_range()),
                                ),
                                parameters: vec![typed::Parameter(mock_node(
                                    ast2::Parameter {
                                        name: str!("zap"),
                                        value_type: Some(typed::TypeExpression(mock_node(
                                            ast2::TypeExpression::Primitive(
                                                ast2::TypePrimitive::Float
                                            ),
                                            TypeRef(TypeShape::Float)
                                        ))),
                                        default_value: Some(typed::Expression(mock_node(
                                            ast2::Expression::Primitive(ast2::Primitive::Float(
                                                1.432, 4
                                            )),
                                            TypeRef(TypeShape::Float)
                                        ))),
                                    },
                                    TypeRef(TypeShape::Float)
                                ))],
                                body: typed::Expression(mock_node(
                                    ast2::Expression::Primitive(ast2::Primitive::Nil),
                                    TypeRef(TypeShape::Nil)
                                )),
                            },
                            TypeRef(TypeShape::View(vec![&TypeRef(TypeShape::Float)]))
                        )),
                        typed::Declaration(mock_node(
                            ast2::Declaration::Module {
                                storage: ast2::Storage::new(
                                    ast2::Visibility::Public,
                                    ast2::Binding::new(str!("my_module"), fixtures::mock_range()),
                                ),
                                value: typed::Module(mock_node(
                                    ast2::Module {
                                        imports: vec![typed::Import(mock_node(
                                            ast2::Import {
                                                source: ast2::ImportSource::Local,
                                                path: vec![str!("buzz")],
                                                alias: Some(str!("Buzz")),
                                            },
                                            TypeRef(TypeShape::Module(vec![(
                                                str!("zip"),
                                                Kind::Type,
                                                &TypeRef(TypeShape::String)
                                            )]))
                                        ))],
                                        declarations: vec![typed::Declaration(mock_node(
                                            ast2::Declaration::TypeAlias {
                                                storage: ast2::Storage::new(
                                                    ast2::Visibility::Public,
                                                    ast2::Binding::new(
                                                        str!("NestedType"),
                                                        fixtures::mock_range()
                                                    ),
                                                ),
                                                value: typed::TypeExpression(mock_node(
                                                    ast2::TypeExpression::Primitive(
                                                        ast2::TypePrimitive::Nil
                                                    ),
                                                    TypeRef(TypeShape::Nil)
                                                )),
                                            },
                                            TypeRef(TypeShape::Nil)
                                        ))],
                                    },
                                    TypeRef(TypeShape::Module(vec![(
                                        str!("NestedType"),
                                        Kind::Type,
                                        &TypeRef(TypeShape::Nil)
                                    )]))
                                )),
                            },
                            TypeRef(TypeShape::Module(vec![(
                                str!("NestedType"),
                                Kind::Type,
                                &TypeRef(TypeShape::Nil)
                            )]))
                        )),
                    ],
                },
                TypeRef(TypeShape::Module(vec![
                    (str!("MyType"), Kind::Type, &TypeRef(TypeShape::Nil)),
                    (
                        str!("MY_CONSTANT"),
                        Kind::Value,
                        &TypeRef(TypeShape::Boolean),
                    ),
                    (str!("MyEnum"), Kind::Value, &enum_type),
                    (str!("my_function"), Kind::Value, &function_type),
                    (str!("MyView"), Kind::Value, &view_type),
                    (str!("my_module"), Kind::Mixed, &module_type),
                ]))
            )))
        );
    }
}
