use crate::ast2::{
    self,
    walk::{NodeId, Visit, Walk},
    Range,
};

#[derive(Debug, PartialEq)]
pub enum Fragment {
    Expression(ast2::Expression<NodeId, NodeId, NodeId>),
    Statement(ast2::Statement<NodeId>),
    Component(ast2::Component<NodeId, NodeId>),
    Parameter(ast2::Parameter<NodeId, NodeId>),
    Declaration(ast2::Declaration<NodeId, NodeId, NodeId, NodeId>),
    TypeExpression(ast2::TypeExpression<NodeId>),
    Import(ast2::Import),
    Module(ast2::Module<NodeId, NodeId>),
}

#[derive(Default)]
pub struct Context {
    next_id: usize,
    fragments: Vec<(NodeId, Fragment)>,
}

impl Context {
    pub fn add(mut self, fragment: Fragment) -> (NodeId, Self) {
        let id = NodeId(self.next_id);
        self.next_id += 1;
        self.fragments.push((id, fragment));
        (id, self)
    }

    pub fn into_fragments(self) -> Vec<(NodeId, Fragment)> {
        self.fragments
    }
}

#[derive(Default)]
struct CollectVisitor {
    next_id: usize,
    fragments: Vec<(NodeId, Fragment)>,
}

impl CollectVisitor {
    pub fn add(mut self, fragment: Fragment) -> (NodeId, Self) {
        let id = NodeId(self.next_id);
        self.next_id += 1;
        self.fragments.push((id, fragment));
        (id, self)
    }

    pub fn into_fragments(self) -> Vec<(NodeId, Fragment)> {
        self.fragments
    }
}

impl Visit for CollectVisitor {
    type Expr = NodeId;

    type Stmt = NodeId;

    type Comp = NodeId;

    type TExpr = NodeId;

    type Param = NodeId;

    type Decl = NodeId;

    type Imp = NodeId;

    type Mod = NodeId;

    fn expression(
        self,
        x: ast2::Expression<Self::Expr, Self::Stmt, Self::Comp>,
        _: Range,
    ) -> (Self::Expr, Self) {
        self.add(Fragment::Expression(x))
    }

    fn statement(self, x: ast2::Statement<Self::Expr>, _: Range) -> (Self::Stmt, Self) {
        self.add(Fragment::Statement(x))
    }

    fn component(self, x: ast2::Component<Self::Expr, Self::Comp>, _: Range) -> (Self::Comp, Self) {
        self.add(Fragment::Component(x))
    }

    fn type_expression(
        self,
        x: ast2::TypeExpression<Self::TExpr>,
        _: Range,
    ) -> (Self::TExpr, Self) {
        self.add(Fragment::TypeExpression(x))
    }

    fn parameter(
        self,
        x: ast2::Parameter<Self::Expr, Self::TExpr>,
        _: Range,
    ) -> (Self::Param, Self) {
        self.add(Fragment::Parameter(x))
    }

    fn declaration(
        self,
        x: ast2::Declaration<Self::Expr, Self::Param, Self::Mod, Self::TExpr>,
        _: Range,
    ) -> (Self::Decl, Self) {
        self.add(Fragment::Declaration(x))
    }

    fn import(self, x: ast2::Import, _: Range) -> (Self::Imp, Self) {
        self.add(Fragment::Import(x))
    }

    fn module(self, x: ast2::Module<Self::Imp, Self::Decl>, _: Range) -> (Self::Mod, Self) {
        self.add(Fragment::Module(x))
    }
}

impl super::Program {
    fn collect(self) -> Vec<(NodeId, Fragment)> {
        let ast2::Program(module) = self.0.value;

        module.walk(CollectVisitor::default()).1.into_fragments()
    }
}

#[cfg(test)]
mod tests {
    use super::Fragment;
    use crate::ast2::{
        self,
        raw::{self, Node},
        walk::NodeId,
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
    fn collect() {
        let program = raw::Program(mock_node(ast2::Program(raw::Module(mock_node(
            ast2::Module {
                imports: vec![raw::Import(mock_node(ast2::Import {
                    source: ast2::ImportSource::Local,
                    path: vec![str!("foo"), str!("bar"), str!("fizz")],
                    alias: None,
                }))],
                declarations: vec![
                    raw::Declaration(mock_node(ast2::Declaration::TypeAlias {
                        storage: ast2::Storage::new(
                            ast2::Visibility::Public,
                            ast2::Binding::new(str!("MyType"), mock_range()),
                        ),
                        value: raw::TypeExpression(mock_node(ast2::TypeExpression::Primitive(
                            ast2::TypePrimitive::Nil,
                        ))),
                    })),
                    raw::Declaration(mock_node(ast2::Declaration::Constant {
                        storage: ast2::Storage::new(
                            ast2::Visibility::Public,
                            ast2::Binding::new(str!("MY_CONSTANT"), mock_range()),
                        ),
                        value_type: Some(raw::TypeExpression(mock_node(
                            ast2::TypeExpression::Primitive(ast2::TypePrimitive::Boolean),
                        ))),
                        value: raw::Expression(mock_node(ast2::Expression::Primitive(
                            ast2::Primitive::Boolean(true),
                        ))),
                    })),
                    raw::Declaration(mock_node(ast2::Declaration::Enumerated {
                        storage: ast2::Storage::new(
                            ast2::Visibility::Public,
                            ast2::Binding::new(str!("MyEnum"), mock_range()),
                        ),
                        variants: vec![
                            (str!("Empty"), vec![]),
                            (
                                str!("Number"),
                                vec![raw::TypeExpression(mock_node(
                                    ast2::TypeExpression::Primitive(ast2::TypePrimitive::Integer),
                                ))],
                            ),
                        ],
                    })),
                    raw::Declaration(mock_node(ast2::Declaration::Function {
                        storage: ast2::Storage::new(
                            ast2::Visibility::Public,
                            ast2::Binding::new(str!("my_function"), mock_range()),
                        ),
                        parameters: vec![raw::Parameter(mock_node(ast2::Parameter {
                            name: str!("zip"),
                            value_type: Some(raw::TypeExpression(mock_node(
                                ast2::TypeExpression::Primitive(ast2::TypePrimitive::String),
                            ))),
                            default_value: Some(raw::Expression(mock_node(
                                ast2::Expression::Primitive(ast2::Primitive::String(str!(
                                    "my string"
                                ))),
                            ))),
                        }))],
                        body_type: Some(raw::TypeExpression(mock_node(
                            ast2::TypeExpression::Primitive(ast2::TypePrimitive::Nil),
                        ))),
                        body: raw::Expression(mock_node(ast2::Expression::Primitive(
                            ast2::Primitive::Nil,
                        ))),
                    })),
                    raw::Declaration(mock_node(ast2::Declaration::View {
                        storage: ast2::Storage::new(
                            ast2::Visibility::Public,
                            ast2::Binding::new(str!("MyView"), mock_range()),
                        ),
                        parameters: vec![raw::Parameter(mock_node(ast2::Parameter {
                            name: str!("zap"),
                            value_type: Some(raw::TypeExpression(mock_node(
                                ast2::TypeExpression::Primitive(ast2::TypePrimitive::Float),
                            ))),
                            default_value: Some(raw::Expression(mock_node(
                                ast2::Expression::Primitive(ast2::Primitive::Float(1.432, 4)),
                            ))),
                        }))],
                        body: raw::Expression(mock_node(ast2::Expression::Primitive(
                            ast2::Primitive::Nil,
                        ))),
                    })),
                    raw::Declaration(mock_node(ast2::Declaration::Module {
                        storage: ast2::Storage::new(
                            ast2::Visibility::Public,
                            ast2::Binding::new(str!("my_module"), mock_range()),
                        ),
                        value: raw::Module(mock_node(ast2::Module {
                            imports: vec![raw::Import(mock_node(ast2::Import {
                                source: ast2::ImportSource::Local,
                                path: vec![str!("buzz")],
                                alias: Some(str!("Buzz")),
                            }))],
                            declarations: vec![raw::Declaration(mock_node(
                                ast2::Declaration::TypeAlias {
                                    storage: ast2::Storage::new(
                                        ast2::Visibility::Public,
                                        ast2::Binding::new(str!("NestedType"), mock_range()),
                                    ),
                                    value: raw::TypeExpression(mock_node(
                                        ast2::TypeExpression::Primitive(ast2::TypePrimitive::Nil),
                                    )),
                                },
                            ))],
                        })),
                    })),
                ],
            },
        )))));

        assert_eq!(
            program.collect(),
            vec![
                (
                    NodeId(0),
                    Fragment::Import(ast2::Import {
                        source: ast2::ImportSource::Local,
                        path: vec![str!("foo"), str!("bar"), str!("fizz")],
                        alias: None,
                    })
                ),
                (
                    NodeId(1),
                    Fragment::TypeExpression(ast2::TypeExpression::Primitive(
                        ast2::TypePrimitive::Nil
                    ))
                ),
                (
                    NodeId(2),
                    Fragment::Declaration(ast2::Declaration::TypeAlias {
                        storage: ast2::Storage::new(
                            ast2::Visibility::Public,
                            ast2::Binding::new(str!("MyType"), mock_range()),
                        ),
                        value: NodeId(1),
                    })
                ),
                (
                    NodeId(3),
                    Fragment::TypeExpression(ast2::TypeExpression::Primitive(
                        ast2::TypePrimitive::Boolean
                    ))
                ),
                (
                    NodeId(4),
                    Fragment::Expression(ast2::Expression::Primitive(ast2::Primitive::Boolean(
                        true
                    ),))
                ),
                (
                    NodeId(5),
                    Fragment::Declaration(ast2::Declaration::Constant {
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
                    Fragment::TypeExpression(ast2::TypeExpression::Primitive(
                        ast2::TypePrimitive::Integer
                    ))
                ),
                (
                    NodeId(7),
                    Fragment::Declaration(ast2::Declaration::Enumerated {
                        storage: ast2::Storage::new(
                            ast2::Visibility::Public,
                            ast2::Binding::new(str!("MyEnum"), mock_range()),
                        ),
                        variants: vec![(str!("Empty"), vec![]), (str!("Number"), vec![NodeId(6)])],
                    })
                ),
                (
                    NodeId(8),
                    Fragment::TypeExpression(ast2::TypeExpression::Primitive(
                        ast2::TypePrimitive::String
                    ))
                ),
                (
                    NodeId(9),
                    Fragment::Expression(ast2::Expression::Primitive(ast2::Primitive::String(
                        str!("my string")
                    )))
                ),
                (
                    NodeId(10),
                    Fragment::Parameter(ast2::Parameter {
                        name: str!("zip"),
                        value_type: Some(NodeId(8)),
                        default_value: Some(NodeId(9)),
                    })
                ),
                (
                    NodeId(11),
                    Fragment::TypeExpression(ast2::TypeExpression::Primitive(
                        ast2::TypePrimitive::Nil
                    ))
                ),
                (
                    NodeId(12),
                    Fragment::Expression(ast2::Expression::Primitive(ast2::Primitive::Nil))
                ),
                (
                    NodeId(13),
                    Fragment::Declaration(ast2::Declaration::Function {
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
                    Fragment::TypeExpression(ast2::TypeExpression::Primitive(
                        ast2::TypePrimitive::Float
                    ))
                ),
                (
                    NodeId(15),
                    Fragment::Expression(ast2::Expression::Primitive(ast2::Primitive::Float(
                        1.432, 4
                    )))
                ),
                (
                    NodeId(16),
                    Fragment::Parameter(ast2::Parameter {
                        name: str!("zap"),
                        value_type: Some(NodeId(14)),
                        default_value: Some(NodeId(15)),
                    })
                ),
                (
                    NodeId(17),
                    Fragment::Expression(ast2::Expression::Primitive(ast2::Primitive::Nil))
                ),
                (
                    NodeId(18),
                    Fragment::Declaration(ast2::Declaration::View {
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
                    Fragment::Import(ast2::Import {
                        source: ast2::ImportSource::Local,
                        path: vec![str!("buzz")],
                        alias: Some(str!("Buzz")),
                    })
                ),
                (
                    NodeId(20),
                    Fragment::TypeExpression(ast2::TypeExpression::Primitive(
                        ast2::TypePrimitive::Nil
                    ))
                ),
                (
                    NodeId(21),
                    Fragment::Declaration(ast2::Declaration::TypeAlias {
                        storage: ast2::Storage::new(
                            ast2::Visibility::Public,
                            ast2::Binding::new(str!("NestedType"), mock_range()),
                        ),
                        value: NodeId(20),
                    })
                ),
                (
                    NodeId(22),
                    Fragment::Module(ast2::Module {
                        imports: vec![NodeId(19)],
                        declarations: vec![NodeId(21)],
                    })
                ),
                (
                    NodeId(23),
                    Fragment::Declaration(ast2::Declaration::Module {
                        storage: ast2::Storage::new(
                            ast2::Visibility::Public,
                            ast2::Binding::new(str!("my_module"), mock_range()),
                        ),
                        value: NodeId(22),
                    })
                ),
                (
                    NodeId(24),
                    Fragment::Module(ast2::Module {
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
