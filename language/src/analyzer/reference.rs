use crate::parser::{
    declaration::{parameter::Parameter, Declaration, DeclarationNode},
    expression::{
        ksx::{KSXNode, KSX},
        statement::Statement,
        Expression, ExpressionNode,
    },
    module::{Module, ModuleNode},
    position::Decrement,
    types::type_expression::{TypeExpression, TypeExpressionNode},
};
use combine::Stream;
use std::fmt::Debug;

impl<T> ExpressionNode<T, i32>
where
    T: Stream<Token = char>,
    T::Position: Copy + Debug + Decrement,
{
    pub fn to_ref(self) -> Expression<i32, i32> {
        match self.0.value() {
            Expression::Primitive(x) => Expression::Primitive(x),

            Expression::Identifier(x) => Expression::Identifier(x),

            Expression::Group(x) => Expression::Group(Box::new(x.0.id())),

            Expression::Closure(xs) => Expression::Closure(
                xs.into_iter()
                    .map(|x| match x {
                        Statement::Effect(x) => Statement::Effect(x.0.id()),
                        Statement::Variable(name, x) => Statement::Variable(name, x.0.id()),
                    })
                    .collect::<Vec<_>>(),
            ),

            Expression::UnaryOperation(op, x) => Expression::UnaryOperation(op, Box::new(x.0.id())),

            Expression::BinaryOperation(op, lhs, rhs) => {
                Expression::BinaryOperation(op, Box::new(lhs.0.id()), Box::new(rhs.0.id()))
            }

            Expression::DotAccess(lhs, rhs) => Expression::DotAccess(Box::new(lhs.0.id()), rhs),

            Expression::FunctionCall(x, args) => Expression::FunctionCall(
                Box::new(x.0.id()),
                args.into_iter().map(|x| x.0.id()).collect::<Vec<_>>(),
            ),

            Expression::Style(xs) => Expression::Style(
                xs.into_iter()
                    .map(|(key, value)| (key, value.0.id()))
                    .collect::<Vec<_>>(),
            ),

            Expression::KSX(x) => Expression::KSX(Box::new(x.0.id())),
        }
    }
}

impl<T> KSXNode<T, i32>
where
    T: Stream<Token = char>,
    T::Position: Copy + Debug + Decrement,
{
    pub fn to_ref(self) -> KSX<i32, i32> {
        let attributes_to_refs = |xs: Vec<(String, Option<ExpressionNode<T, i32>>)>| {
            xs.into_iter()
                .map(|(key, value)| (key, value.map(|x| x.0.id())))
                .collect::<Vec<_>>()
        };

        match self.0.value() {
            KSX::Text(x) => KSX::Text(x),

            KSX::Inline(x) => KSX::Inline(x.0.id()),

            KSX::Fragment(xs) => {
                KSX::Fragment(xs.into_iter().map(|x| x.0.id()).collect::<Vec<_>>())
            }

            KSX::ClosedElement(tag, attributes) => {
                KSX::ClosedElement(tag, attributes_to_refs(attributes))
            }

            KSX::OpenElement(start_tag, attributes, children, end_tag) => KSX::OpenElement(
                start_tag,
                attributes_to_refs(attributes),
                children.into_iter().map(|x| x.0.id()).collect::<Vec<_>>(),
                end_tag,
            ),
        }
    }
}

impl<T> TypeExpressionNode<T, i32>
where
    T: Stream<Token = char>,
    T::Position: Copy + Debug + Decrement,
{
    pub fn to_ref(self) -> TypeExpression<i32> {
        match self.0.value() {
            TypeExpression::Nil => TypeExpression::Nil,
            TypeExpression::Boolean => TypeExpression::Boolean,
            TypeExpression::Integer => TypeExpression::Integer,
            TypeExpression::Float => TypeExpression::Float,
            TypeExpression::String => TypeExpression::String,
            TypeExpression::Style => TypeExpression::Style,
            TypeExpression::Element => TypeExpression::Element,

            TypeExpression::Identifier(x) => TypeExpression::Identifier(x),

            TypeExpression::Group(x) => TypeExpression::Group(Box::new((*x).0.id())),

            TypeExpression::DotAccess(lhs, rhs) => {
                TypeExpression::DotAccess(Box::new((*lhs).0.id()), rhs)
            }

            TypeExpression::Function(params, body) => TypeExpression::Function(
                params.into_iter().map(|x| x.0.id()).collect::<Vec<_>>(),
                Box::new(body.0.id()),
            ),
        }
    }
}

impl<T> DeclarationNode<T, i32>
where
    T: Stream<Token = char>,
    T::Position: Copy + Debug + Decrement,
{
    pub fn to_ref(self) -> Declaration<i32, i32, i32> {
        let parameters_to_refs =
            |xs: Vec<Parameter<ExpressionNode<T, i32>, TypeExpressionNode<T, i32>>>| {
                xs.into_iter()
                    .map(
                        |Parameter {
                             name,
                             value_type,
                             default_value,
                         }| Parameter {
                            name,
                            value_type: value_type.map(|x| x.0.id()),
                            default_value: default_value.map(|x| x.0.id()),
                        },
                    )
                    .collect::<Vec<_>>()
            };

        match self.0.value() {
            Declaration::TypeAlias { name, value } => Declaration::TypeAlias {
                name,
                value: value.0.id(),
            },

            Declaration::Enumerated { name, variants } => Declaration::Enumerated {
                name,
                variants: variants
                    .into_iter()
                    .map(|(name, params)| {
                        (
                            name,
                            params.into_iter().map(|x| x.0.id()).collect::<Vec<_>>(),
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
                value_type: value_type.map(|x| x.0.id()),
                value: value.0.id(),
            },

            Declaration::Function {
                name,
                parameters,
                body_type,
                body,
            } => Declaration::Function {
                name,
                parameters: parameters_to_refs(parameters),
                body_type: body_type.map(|x| x.0.id()),
                body: body.0.id(),
            },

            Declaration::View {
                name,
                parameters,
                body,
            } => Declaration::View {
                name,
                parameters: parameters_to_refs(parameters),
                body: body.0.id(),
            },

            Declaration::Module { name, value } => Declaration::Module {
                name,
                value: value.id(),
            },
        }
    }
}

impl<T> ModuleNode<T, i32>
where
    T: Stream<Token = char>,
    T::Position: Copy + Debug + Decrement,
{
    pub fn id(self) -> i32 {
        self.1
    }

    pub fn to_ref(self) -> Module<i32> {
        Module {
            imports: self.0.imports,
            declarations: self
                .0
                .declarations
                .into_iter()
                .map(|x| x.0.id())
                .collect::<Vec<_>>(),
        }
    }
}

#[cfg(test)]
mod tests {
    use crate::parser::{
        declaration::{
            parameter::Parameter,
            storage::{Storage, Visibility},
            Declaration, DeclarationNode,
        },
        expression::{
            binary_operation::BinaryOperator,
            ksx::{KSXNode, KSX},
            primitive::Primitive,
            statement::Statement,
            Expression, ExpressionNode, UnaryOperator,
        },
        module::{
            import::{Import, Source},
            Module, ModuleNode,
        },
        node::Node,
        range::Range,
        types::type_expression::{TypeExpression, TypeExpressionNode},
        CharStream,
    };

    const RANGE: Range<CharStream> = Range::chars((1, 1), (1, 1));

    #[test]
    fn expression_primitive() {
        let input = ExpressionNode(Node(Expression::Primitive(Primitive::Nil), RANGE, 0));

        assert_eq!(input.to_ref(), Expression::Primitive(Primitive::Nil))
    }

    #[test]
    fn expression_identifier() {
        let input = ExpressionNode(Node(Expression::Identifier(String::from("foo")), RANGE, 0));

        assert_eq!(input.to_ref(), Expression::Identifier(String::from("foo")))
    }

    #[test]
    fn expression_group() {
        let input = ExpressionNode(Node(
            Expression::Group(Box::new(ExpressionNode(Node(
                Expression::Primitive(Primitive::Nil),
                RANGE,
                0,
            )))),
            RANGE,
            1,
        ));

        assert_eq!(input.to_ref(), Expression::Group(Box::new(0)))
    }

    #[test]
    fn expression_closure() {
        let input = ExpressionNode(Node(
            Expression::Closure(vec![
                Statement::Variable(
                    String::from("foo"),
                    ExpressionNode(Node(Expression::Primitive(Primitive::Nil), RANGE, 0)),
                ),
                Statement::Effect(ExpressionNode(Node(
                    Expression::Primitive(Primitive::Nil),
                    RANGE,
                    1,
                ))),
            ]),
            RANGE,
            2,
        ));

        assert_eq!(
            input.to_ref(),
            Expression::Closure(vec![
                Statement::Variable(String::from("foo"), 0),
                Statement::Effect(1),
            ])
        )
    }

    #[test]
    fn expression_unary_operation() {
        let input = ExpressionNode(Node(
            Expression::UnaryOperation(
                UnaryOperator::Not,
                Box::new(ExpressionNode(Node(
                    Expression::Primitive(Primitive::Nil),
                    RANGE,
                    0,
                ))),
            ),
            RANGE,
            1,
        ));

        assert_eq!(
            input.to_ref(),
            Expression::UnaryOperation(UnaryOperator::Not, Box::new(0))
        )
    }

    #[test]
    fn expression_binary_operation() {
        let input: ExpressionNode<
            combine::easy::Stream<
                combine::stream::position::Stream<&str, combine::stream::position::SourcePosition>,
            >,
            i32,
        > = ExpressionNode(Node(
            Expression::BinaryOperation(
                BinaryOperator::Equal,
                Box::new(ExpressionNode(Node(
                    Expression::Primitive(Primitive::Nil),
                    RANGE,
                    0,
                ))),
                Box::new(ExpressionNode(Node(
                    Expression::Primitive(Primitive::Nil),
                    RANGE,
                    1,
                ))),
            ),
            RANGE,
            2,
        ));

        assert_eq!(
            input.to_ref(),
            Expression::BinaryOperation(BinaryOperator::Equal, Box::new(0), Box::new(1))
        )
    }

    #[test]
    fn expression_dot_access() {
        let input = ExpressionNode(Node(
            Expression::DotAccess(
                Box::new(ExpressionNode(Node(
                    Expression::Primitive(Primitive::Nil),
                    RANGE,
                    0,
                ))),
                String::from("foo"),
            ),
            RANGE,
            1,
        ));

        assert_eq!(
            input.to_ref(),
            Expression::DotAccess(Box::new(0), String::from("foo"))
        )
    }

    #[test]
    fn expression_function_call() {
        let input = ExpressionNode(Node(
            Expression::FunctionCall(
                Box::new(ExpressionNode(Node(
                    Expression::Primitive(Primitive::Nil),
                    RANGE,
                    0,
                ))),
                vec![
                    ExpressionNode(Node(Expression::Primitive(Primitive::Nil), RANGE, 1)),
                    ExpressionNode(Node(Expression::Primitive(Primitive::Nil), RANGE, 2)),
                ],
            ),
            RANGE,
            3,
        ));

        assert_eq!(
            input.to_ref(),
            Expression::FunctionCall(Box::new(0), vec![1, 2])
        )
    }

    #[test]
    fn expression_style() {
        let input = ExpressionNode(Node(
            Expression::Style(vec![
                (
                    String::from("foo"),
                    ExpressionNode(Node(Expression::Primitive(Primitive::Nil), RANGE, 0)),
                ),
                (
                    String::from("bar"),
                    ExpressionNode(Node(Expression::Primitive(Primitive::Nil), RANGE, 1)),
                ),
            ]),
            RANGE,
            2,
        ));

        assert_eq!(
            input.to_ref(),
            Expression::Style(vec![(String::from("foo"), 0), (String::from("bar"), 1)])
        )
    }

    #[test]
    fn expression_ksx() {
        let input = ExpressionNode(Node(
            Expression::KSX(Box::new(KSXNode(Node(KSX::Fragment(vec![]), RANGE, 0)))),
            RANGE,
            1,
        ));

        assert_eq!(input.to_ref(), Expression::KSX(Box::new(0)))
    }

    #[test]
    fn ksx_text() {
        let input = KSXNode(Node(KSX::Text(String::from("foo")), RANGE, 0));

        assert_eq!(input.to_ref(), KSX::Text(String::from("foo")))
    }

    #[test]
    fn ksx_inline() {
        let input = KSXNode(Node(
            KSX::Inline(ExpressionNode(Node(
                Expression::Primitive(Primitive::Nil),
                RANGE,
                0,
            ))),
            RANGE,
            1,
        ));

        assert_eq!(input.to_ref(), KSX::Inline(0))
    }

    #[test]
    fn ksx_fragment() {
        let input = KSXNode(Node(
            KSX::Fragment(vec![KSXNode(Node(
                KSX::Text(String::from("foo")),
                RANGE,
                0,
            ))]),
            RANGE,
            1,
        ));

        assert_eq!(input.to_ref(), KSX::Fragment(vec![0]))
    }

    #[test]
    fn ksx_closed_element() {
        let input = KSXNode(Node(
            KSX::ClosedElement(
                String::from("Foo"),
                vec![
                    (String::from("bar"), None),
                    (
                        String::from("fizz"),
                        Some(ExpressionNode(Node(
                            Expression::Primitive(Primitive::Nil),
                            RANGE,
                            0,
                        ))),
                    ),
                ],
            ),
            RANGE,
            2,
        ));

        assert_eq!(
            input.to_ref(),
            KSX::ClosedElement(
                String::from("Foo"),
                vec![(String::from("bar"), None), (String::from("fizz"), Some(0))],
            )
        )
    }

    #[test]
    fn ksx_open_element() {
        let input = KSXNode(Node(
            KSX::OpenElement(
                String::from("Foo"),
                vec![
                    (String::from("bar"), None),
                    (
                        String::from("fizz"),
                        Some(ExpressionNode(Node(
                            Expression::Primitive(Primitive::Nil),
                            RANGE,
                            0,
                        ))),
                    ),
                ],
                vec![KSXNode(Node(KSX::Text(String::from("buzz")), RANGE, 1))],
                String::from("Foo"),
            ),
            RANGE,
            2,
        ));

        assert_eq!(
            input.to_ref(),
            KSX::OpenElement(
                String::from("Foo"),
                vec![(String::from("bar"), None), (String::from("fizz"), Some(0))],
                vec![1],
                String::from("Foo")
            )
        )
    }

    #[test]
    fn declaration_type_alias() {
        let input = DeclarationNode(Node(
            Declaration::TypeAlias {
                name: Storage(Visibility::Public, String::from("FOO")),
                value: TypeExpressionNode(Node(TypeExpression::Nil, RANGE, 0)),
            },
            RANGE,
            2,
        ));

        assert_eq!(
            input.to_ref(),
            Declaration::TypeAlias {
                name: Storage(Visibility::Public, String::from("FOO")),
                value: 0
            }
        )
    }

    #[test]
    fn declaration_enumerated() {
        let input = DeclarationNode(Node(
            Declaration::Enumerated {
                name: Storage(Visibility::Public, String::from("Foo")),
                variants: vec![(
                    String::from("Bar"),
                    vec![TypeExpressionNode(Node(TypeExpression::Nil, RANGE, 0))],
                )],
            },
            RANGE,
            1,
        ));

        assert_eq!(
            input.to_ref(),
            Declaration::Enumerated {
                name: Storage(Visibility::Public, String::from("Foo")),
                variants: vec![(String::from("Bar"), vec![0])]
            }
        )
    }

    #[test]
    fn declaration_constant() {
        let input = DeclarationNode(Node(
            Declaration::Constant {
                name: Storage(Visibility::Public, String::from("FOO")),
                value_type: Some(TypeExpressionNode(Node(TypeExpression::Nil, RANGE, 0))),
                value: ExpressionNode(Node(Expression::Primitive(Primitive::Nil), RANGE, 1)),
            },
            RANGE,
            2,
        ));

        assert_eq!(
            input.to_ref(),
            Declaration::Constant {
                name: Storage(Visibility::Public, String::from("FOO")),
                value_type: Some(0),
                value: 1
            }
        )
    }

    #[test]
    fn declaration_function() {
        let input = DeclarationNode(Node(
            Declaration::Function {
                name: Storage(Visibility::Public, String::from("foo")),
                parameters: vec![Parameter {
                    name: String::from("bar"),
                    value_type: Some(TypeExpressionNode(Node(TypeExpression::Nil, RANGE, 0))),
                    default_value: Some(ExpressionNode(Node(
                        Expression::Primitive(Primitive::Nil),
                        RANGE,
                        1,
                    ))),
                }],
                body_type: Some(TypeExpressionNode(Node(TypeExpression::Nil, RANGE, 2))),
                body: ExpressionNode(Node(Expression::Primitive(Primitive::Nil), RANGE, 3)),
            },
            RANGE,
            4,
        ));

        assert_eq!(
            input.to_ref(),
            Declaration::Function {
                name: Storage(Visibility::Public, String::from("foo")),
                parameters: vec![Parameter {
                    name: String::from("bar"),
                    value_type: Some(0),
                    default_value: Some(1),
                }],
                body_type: Some(2),
                body: 3
            }
        )
    }

    #[test]
    fn declaration_view() {
        let input = DeclarationNode(Node(
            Declaration::View {
                name: Storage(Visibility::Public, String::from("foo")),
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
        ));

        assert_eq!(
            input.to_ref(),
            Declaration::View {
                name: Storage(Visibility::Public, String::from("foo")),
                parameters: vec![Parameter {
                    name: String::from("bar"),
                    value_type: Some(0),
                    default_value: Some(1),
                }],
                body: 2
            }
        )
    }

    #[test]
    fn declaration_module() {
        let input = DeclarationNode(Node(
            Declaration::Module {
                name: Storage(Visibility::Public, String::from("foo")),
                value: ModuleNode(
                    Module {
                        imports: vec![],
                        declarations: vec![],
                    },
                    0,
                ),
            },
            RANGE,
            1,
        ));

        assert_eq!(
            input.to_ref(),
            Declaration::Module {
                name: Storage(Visibility::Public, String::from("foo")),
                value: 0,
            }
        )
    }

    #[test]
    fn module() {
        let input = ModuleNode(
            Module {
                imports: vec![Import::new(Source::Root, vec![String::from("foo")], None)],
                declarations: vec![DeclarationNode(Node(
                    Declaration::Constant {
                        name: Storage(Visibility::Public, String::from("FOO")),
                        value_type: Some(TypeExpressionNode(Node(TypeExpression::Nil, RANGE, 0))),
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
        );

        assert_eq!(
            input.to_ref(),
            Module {
                imports: vec![Import::new(Source::Root, vec![String::from("foo")], None)],
                declarations: vec![2],
            }
        )
    }
}
