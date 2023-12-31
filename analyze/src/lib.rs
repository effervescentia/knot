mod context;
mod declaration;
mod expression;
mod fragment;
mod import;
mod infer;
mod ksx;
mod module;
mod parameter;
mod register;
mod statement;
#[cfg(test)]
mod test;
mod type_expression;
mod types;

use context::{FileContext, NodeContext, ScopeContext, StrongContext, WeakContext};
pub use infer::strong::Strong;
use infer::strong::{SemanticError, ToStrong};
use lang::Program;
use register::Register;
use std::{cell::RefCell, fmt::Debug};
use types::Type;

#[derive(Clone, Debug, PartialEq)]
pub struct FinalType(Result<Type<Box<FinalType>>, SemanticError>);

#[derive(Clone, Debug, PartialEq)]
pub struct PreviewType(Type<Box<PreviewType>>);

#[derive(Clone, Copy, Debug, PartialEq)]
pub enum RefKind {
    Type,
    Value,
    Mixed,
}

fn register_fragments<R>(x: &Program<R, ()>) -> (Program<R, NodeContext>, FileContext)
where
    R: Copy,
{
    let file_ctx = RefCell::new(FileContext::new());
    let untyped = x.0.register(&ScopeContext::new(&file_ctx));

    (Program(untyped), file_ctx.into_inner())
}

pub fn analyze<R>(x: &Program<R, ()>) -> Program<R, Strong>
where
    R: Copy,
{
    // register AST fragments depth-first with monotonically increasing IDs
    let (untyped, file_ctx) = register_fragments(x);

    // apply weak type inference
    let WeakContext {
        bindings,
        fragments,
        refs: weak_refs,
    } = infer::weak::infer_types(file_ctx);

    // apply strong type inference
    let nodes = fragments.to_descriptors(weak_refs);
    let strong_ctx = infer::strong::infer_types(&nodes, fragments, bindings);

    untyped.to_strong(&strong_ctx)
}

impl<R> ToStrong<Program<R, Strong>> for Program<R, NodeContext>
where
    R: Copy,
{
    fn to_strong(&self, ctx: &StrongContext) -> Program<R, Strong> {
        Program(self.0.to_strong(ctx))
    }
}

#[cfg(test)]
mod tests {
    use crate::{test::fixture as f, types::Type, RefKind};
    use kore::str;
    use lang::{
        ast::{Expression, Module, ModuleNode, Primitive, TypeExpression},
        Program,
    };

    #[test]
    fn empty_module() {
        let ast = Program(f::n::m(Module::new(vec![], vec![])));

        assert_eq!(
            super::analyze(&ast),
            Program(ModuleNode(
                Module::new(vec![], vec![]),
                Ok(Type::Module(vec![]))
            ))
        );
    }

    mod type_alias {
        use super::*;

        #[test]
        fn primitive() {
            let ast = Program(f::n::mr(Module::new(
                vec![],
                vec![f::n::d(f::a::const_(
                    "foo",
                    None,
                    f::n::x(Expression::Primitive(Primitive::Nil)),
                ))],
            )));

            assert_eq!(
                super::super::analyze(&ast),
                Program(ModuleNode(
                    Module::new(
                        vec![],
                        vec![f::n::dc(
                            f::a::const_(
                                "foo",
                                None,
                                f::n::xc(Expression::Primitive(Primitive::Nil), Ok(Type::Nil))
                            ),
                            Ok(Type::Nil)
                        )]
                    ),
                    Ok(Type::Module(vec![(str!("foo"), RefKind::Value, 1)]))
                ))
            );
        }
    }

    mod enumerated {
        use super::*;

        #[test]
        fn static_variant() {
            let ast = Program(f::n::mr(Module::new(
                vec![],
                vec![f::n::d(f::a::enum_("foo", vec![(str!("Bar"), vec![])]))],
            )));

            assert_eq!(
                super::super::analyze(&ast),
                Program(ModuleNode(
                    Module::new(
                        vec![],
                        vec![f::n::dc(
                            f::a::enum_("foo", vec![(str!("Bar"), vec![])]),
                            Ok(Type::Enumerated(vec![(str!("Bar"), vec![])]))
                        )]
                    ),
                    Ok(Type::Module(vec![(str!("foo"), RefKind::Mixed, 0)]))
                ))
            );
        }

        #[test]
        fn parameterized_variant() {
            let ast = Program(f::n::mr(Module::new(
                vec![],
                vec![f::n::d(f::a::enum_(
                    "foo",
                    vec![(
                        str!("Bar"),
                        vec![
                            f::n::tx(TypeExpression::Nil),
                            f::n::tx(TypeExpression::Boolean),
                        ],
                    )],
                ))],
            )));

            assert_eq!(
                super::super::analyze(&ast),
                Program(ModuleNode(
                    Module::new(
                        vec![],
                        vec![f::n::dc(
                            f::a::enum_(
                                "foo",
                                vec![(
                                    str!("Bar"),
                                    vec![
                                        f::n::txc(TypeExpression::Nil, Ok(Type::Nil)),
                                        f::n::txc(TypeExpression::Boolean, Ok(Type::Boolean))
                                    ]
                                )]
                            ),
                            Ok(Type::Enumerated(vec![(str!("Bar"), vec![0, 1])]))
                        )]
                    ),
                    Ok(Type::Module(vec![(str!("foo"), RefKind::Mixed, 2)]))
                ))
            );
        }
    }

    mod constant {
        use super::*;

        #[test]
        fn without_typedef() {
            let ast = Program(f::n::mr(Module::new(
                vec![],
                vec![f::n::d(f::a::const_(
                    "foo",
                    None,
                    f::n::x(Expression::Primitive(Primitive::Nil)),
                ))],
            )));

            assert_eq!(
                super::super::analyze(&ast),
                Program(ModuleNode(
                    Module::new(
                        vec![],
                        vec![f::n::dc(
                            f::a::const_(
                                "foo",
                                None,
                                f::n::xc(Expression::Primitive(Primitive::Nil), Ok(Type::Nil))
                            ),
                            Ok(Type::Nil)
                        )]
                    ),
                    Ok(Type::Module(vec![(str!("foo"), RefKind::Value, 1)]))
                ))
            );
        }

        #[test]
        fn with_typedef() {
            let ast = Program(f::n::mr(Module::new(
                vec![],
                vec![f::n::d(f::a::const_(
                    "foo",
                    Some(f::n::tx(TypeExpression::Nil)),
                    f::n::x(Expression::Primitive(Primitive::Boolean(true))),
                ))],
            )));

            assert_eq!(
                super::super::analyze(&ast),
                Program(ModuleNode(
                    Module::new(
                        vec![],
                        vec![f::n::dc(
                            f::a::const_(
                                "foo",
                                Some(f::n::txc(TypeExpression::Nil, Ok(Type::Nil))),
                                f::n::xc(
                                    Expression::Primitive(Primitive::Boolean(true)),
                                    Ok(Type::Boolean)
                                )
                            ),
                            Ok(Type::Nil)
                        )]
                    ),
                    Ok(Type::Module(vec![(str!("foo"), RefKind::Value, 2)]))
                ))
            );
        }
    }

    mod function {
        use super::*;
        use lang::ast::{BinaryOperator, Parameter};

        #[test]
        fn no_parameters() {
            let ast = Program(f::n::mr(Module::new(
                vec![],
                vec![f::n::d(f::a::func_(
                    "foo",
                    vec![],
                    None,
                    f::n::x(Expression::Primitive(Primitive::Nil)),
                ))],
            )));

            assert_eq!(
                super::super::analyze(&ast),
                Program(ModuleNode(
                    Module::new(
                        vec![],
                        vec![f::n::dc(
                            f::a::func_(
                                "foo",
                                vec![],
                                None,
                                f::n::xc(Expression::Primitive(Primitive::Nil), Ok(Type::Nil))
                            ),
                            Ok(Type::Function(vec![], 0))
                        )]
                    ),
                    Ok(Type::Module(vec![(str!("foo"), RefKind::Value, 1)]))
                ))
            );
        }

        #[test]
        fn parameters_with_defaults() {
            let ast = Program(f::n::mr(Module::new(
                vec![],
                vec![f::n::d(f::a::func_(
                    "foo",
                    vec![
                        f::n::p(Parameter {
                            name: str!("bar"),
                            value_type: None,
                            default_value: Some(f::n::x(Expression::Primitive(
                                Primitive::Boolean(true),
                            ))),
                        }),
                        f::n::p(Parameter {
                            name: str!("fizz"),
                            value_type: None,
                            default_value: Some(f::n::x(Expression::Primitive(
                                Primitive::Integer(123),
                            ))),
                        }),
                    ],
                    None,
                    f::n::x(Expression::Primitive(Primitive::Nil)),
                ))],
            )));

            assert_eq!(
                super::super::analyze(&ast),
                Program(ModuleNode(
                    Module::new(
                        vec![],
                        vec![f::n::dc(
                            f::a::func_(
                                "foo",
                                vec![
                                    f::n::pc(
                                        Parameter {
                                            name: str!("bar"),
                                            value_type: None,
                                            default_value: Some(f::n::xc(
                                                Expression::Primitive(Primitive::Boolean(true),),
                                                Ok(Type::Boolean)
                                            )),
                                        },
                                        Ok(Type::Boolean)
                                    ),
                                    f::n::pc(
                                        Parameter {
                                            name: str!("fizz"),
                                            value_type: None,
                                            default_value: Some(f::n::xc(
                                                Expression::Primitive(Primitive::Integer(123)),
                                                Ok(Type::Integer)
                                            )),
                                        },
                                        Ok(Type::Integer)
                                    ),
                                ],
                                None,
                                f::n::xc(Expression::Primitive(Primitive::Nil), Ok(Type::Nil))
                            ),
                            Ok(Type::Function(vec![1, 3], 4))
                        )]
                    ),
                    Ok(Type::Module(vec![(str!("foo"), RefKind::Value, 5)]))
                ))
            );
        }

        #[test]
        fn parameters_with_typedefs() {
            let ast = Program(f::n::mr(Module::new(
                vec![],
                vec![f::n::d(f::a::func_(
                    "foo",
                    vec![
                        f::n::p(Parameter {
                            name: str!("bar"),
                            value_type: Some(f::n::tx(TypeExpression::Boolean)),
                            default_value: None,
                        }),
                        f::n::p(Parameter {
                            name: str!("fizz"),
                            value_type: Some(f::n::tx(TypeExpression::Integer)),
                            default_value: None,
                        }),
                    ],
                    None,
                    f::n::x(Expression::Primitive(Primitive::Nil)),
                ))],
            )));

            assert_eq!(
                super::super::analyze(&ast),
                Program(ModuleNode(
                    Module::new(
                        vec![],
                        vec![f::n::dc(
                            f::a::func_(
                                "foo",
                                vec![
                                    f::n::pc(
                                        Parameter {
                                            name: str!("bar"),
                                            value_type: Some(f::n::txc(
                                                TypeExpression::Boolean,
                                                Ok(Type::Boolean)
                                            )),
                                            default_value: None,
                                        },
                                        Ok(Type::Boolean)
                                    ),
                                    f::n::pc(
                                        Parameter {
                                            name: str!("fizz"),
                                            value_type: Some(f::n::txc(
                                                TypeExpression::Integer,
                                                Ok(Type::Integer)
                                            )),
                                            default_value: None,
                                        },
                                        Ok(Type::Integer)
                                    ),
                                ],
                                None,
                                f::n::xc(Expression::Primitive(Primitive::Nil), Ok(Type::Nil))
                            ),
                            Ok(Type::Function(vec![1, 3], 4))
                        )]
                    ),
                    Ok(Type::Module(vec![(str!("foo"), RefKind::Value, 5)]))
                ))
            );
        }

        #[test]
        fn parameter_with_default_and_typedef() {
            let ast = Program(f::n::mr(Module::new(
                vec![],
                vec![f::n::d(f::a::func_(
                    "foo",
                    vec![f::n::p(Parameter {
                        name: str!("bar"),
                        value_type: Some(f::n::tx(TypeExpression::Boolean)),
                        default_value: Some(f::n::x(Expression::Primitive(Primitive::Integer(
                            123,
                        )))),
                    })],
                    None,
                    f::n::x(Expression::Primitive(Primitive::Nil)),
                ))],
            )));

            assert_eq!(
                super::super::analyze(&ast),
                Program(ModuleNode(
                    Module::new(
                        vec![],
                        vec![f::n::dc(
                            f::a::func_(
                                "foo",
                                vec![f::n::pc(
                                    Parameter {
                                        name: str!("bar"),
                                        value_type: Some(f::n::txc(
                                            TypeExpression::Boolean,
                                            Ok(Type::Boolean)
                                        )),
                                        default_value: Some(f::n::xc(
                                            Expression::Primitive(Primitive::Integer(123)),
                                            Ok(Type::Integer)
                                        )),
                                    },
                                    Ok(Type::Boolean)
                                )],
                                None,
                                f::n::xc(Expression::Primitive(Primitive::Nil), Ok(Type::Nil))
                            ),
                            Ok(Type::Function(vec![2], 3))
                        )]
                    ),
                    Ok(Type::Module(vec![(str!("foo"), RefKind::Value, 4)]))
                ))
            );
        }

        #[test]
        fn infer_return_type_from_parameter() {
            let ast = Program(f::n::mr(Module::new(
                vec![],
                vec![f::n::d(f::a::func_(
                    "foo",
                    vec![f::n::p(Parameter {
                        name: str!("bar"),
                        value_type: Some(f::n::tx(TypeExpression::Boolean)),
                        default_value: None,
                    })],
                    None,
                    f::n::x(Expression::Identifier(str!("bar"))),
                ))],
            )));

            assert_eq!(
                super::super::analyze(&ast),
                Program(ModuleNode(
                    Module::new(
                        vec![],
                        vec![f::n::dc(
                            f::a::func_(
                                "foo",
                                vec![f::n::pc(
                                    Parameter {
                                        name: str!("bar"),
                                        value_type: Some(f::n::txc(
                                            TypeExpression::Boolean,
                                            Ok(Type::Boolean)
                                        )),
                                        default_value: None,
                                    },
                                    Ok(Type::Boolean)
                                )],
                                None,
                                f::n::xc(Expression::Identifier(str!("bar")), Ok(Type::Boolean))
                            ),
                            Ok(Type::Function(vec![1], 2))
                        )]
                    ),
                    Ok(Type::Module(vec![(str!("foo"), RefKind::Value, 3)]))
                ))
            );
        }

        #[test]
        fn with_return_type() {
            let ast = Program(f::n::mr(Module::new(
                vec![],
                vec![f::n::d(f::a::func_(
                    "foo",
                    vec![],
                    Some(f::n::tx(TypeExpression::Boolean)),
                    f::n::x(Expression::Primitive(Primitive::Nil)),
                ))],
            )));

            assert_eq!(
                super::super::analyze(&ast),
                Program(ModuleNode(
                    Module::new(
                        vec![],
                        vec![f::n::dc(
                            f::a::func_(
                                "foo",
                                vec![],
                                Some(f::n::txc(TypeExpression::Boolean, Ok(Type::Boolean))),
                                f::n::xc(Expression::Primitive(Primitive::Nil), Ok(Type::Nil)),
                            ),
                            Ok(Type::Function(vec![], 0))
                        )]
                    ),
                    Ok(Type::Module(vec![(str!("foo"), RefKind::Value, 2)]))
                ))
            );
        }

        #[test]
        #[ignore = "todo"]
        fn infer_parameters() {
            let ast = Program(f::n::mr(Module::new(
                vec![],
                vec![f::n::d(f::a::func_(
                    "foo",
                    vec![
                        f::n::p(Parameter {
                            name: str!("bar"),
                            value_type: None,
                            default_value: None,
                        }),
                        f::n::p(Parameter {
                            name: str!("fizz"),
                            value_type: None,
                            default_value: None,
                        }),
                    ],
                    None,
                    f::n::x(Expression::BinaryOperation(
                        BinaryOperator::And,
                        Box::new(f::n::x(Expression::Identifier(str!("bar")))),
                        Box::new(f::n::x(Expression::Identifier(str!("fizz")))),
                    )),
                ))],
            )));

            assert_eq!(
                super::super::analyze(&ast),
                Program(ModuleNode(
                    Module::new(
                        vec![],
                        vec![f::n::dc(
                            f::a::func_(
                                "foo",
                                vec![
                                    f::n::pc(
                                        Parameter {
                                            name: str!("bar"),
                                            value_type: None,
                                            default_value: None,
                                        },
                                        Ok(Type::Boolean)
                                    ),
                                    f::n::pc(
                                        Parameter {
                                            name: str!("fizz"),
                                            value_type: None,
                                            default_value: None,
                                        },
                                        Ok(Type::Boolean)
                                    ),
                                ],
                                None,
                                f::n::xc(
                                    Expression::BinaryOperation(
                                        BinaryOperator::And,
                                        Box::new(f::n::xc(
                                            Expression::Identifier(str!("bar")),
                                            Ok(Type::Boolean)
                                        )),
                                        Box::new(f::n::xc(
                                            Expression::Identifier(str!("fizz")),
                                            Ok(Type::Boolean)
                                        )),
                                    ),
                                    Ok(Type::Boolean)
                                )
                            ),
                            Ok(Type::Function(vec![0, 1], 2))
                        )]
                    ),
                    Ok(Type::Module(vec![(str!("foo"), RefKind::Value, 3)]))
                ))
            );
        }
    }

    mod view {
        use super::*;
        use lang::ast::{BinaryOperator, Parameter};

        #[test]
        fn no_parameters() {
            let ast = Program(f::n::mr(Module::new(
                vec![],
                vec![f::n::d(f::a::view(
                    "foo",
                    vec![],
                    f::n::x(Expression::Primitive(Primitive::Nil)),
                ))],
            )));

            assert_eq!(
                super::super::analyze(&ast),
                Program(ModuleNode(
                    Module::new(
                        vec![],
                        vec![f::n::dc(
                            f::a::view(
                                "foo",
                                vec![],
                                f::n::xc(Expression::Primitive(Primitive::Nil), Ok(Type::Nil))
                            ),
                            Ok(Type::View(vec![]))
                        )]
                    ),
                    Ok(Type::Module(vec![(str!("foo"), RefKind::Value, 1)]))
                ))
            );
        }

        #[test]
        fn parameters_with_defaults() {
            let ast = Program(f::n::mr(Module::new(
                vec![],
                vec![f::n::d(f::a::view(
                    "foo",
                    vec![
                        f::n::p(Parameter {
                            name: str!("bar"),
                            value_type: None,
                            default_value: Some(f::n::x(Expression::Primitive(
                                Primitive::Boolean(true),
                            ))),
                        }),
                        f::n::p(Parameter {
                            name: str!("fizz"),
                            value_type: None,
                            default_value: Some(f::n::x(Expression::Primitive(
                                Primitive::Integer(123),
                            ))),
                        }),
                    ],
                    f::n::x(Expression::Primitive(Primitive::Nil)),
                ))],
            )));

            assert_eq!(
                super::super::analyze(&ast),
                Program(ModuleNode(
                    Module::new(
                        vec![],
                        vec![f::n::dc(
                            f::a::view(
                                "foo",
                                vec![
                                    f::n::pc(
                                        Parameter {
                                            name: str!("bar"),
                                            value_type: None,
                                            default_value: Some(f::n::xc(
                                                Expression::Primitive(Primitive::Boolean(true),),
                                                Ok(Type::Boolean)
                                            )),
                                        },
                                        Ok(Type::Boolean)
                                    ),
                                    f::n::pc(
                                        Parameter {
                                            name: str!("fizz"),
                                            value_type: None,
                                            default_value: Some(f::n::xc(
                                                Expression::Primitive(Primitive::Integer(123)),
                                                Ok(Type::Integer)
                                            )),
                                        },
                                        Ok(Type::Integer)
                                    ),
                                ],
                                f::n::xc(Expression::Primitive(Primitive::Nil), Ok(Type::Nil))
                            ),
                            Ok(Type::View(vec![1, 3]))
                        )]
                    ),
                    Ok(Type::Module(vec![(str!("foo"), RefKind::Value, 5)]))
                ))
            );
        }

        #[test]
        fn parameters_with_typedefs() {
            let ast = Program(f::n::mr(Module::new(
                vec![],
                vec![f::n::d(f::a::view(
                    "foo",
                    vec![
                        f::n::p(Parameter {
                            name: str!("bar"),
                            value_type: Some(f::n::tx(TypeExpression::Boolean)),
                            default_value: None,
                        }),
                        f::n::p(Parameter {
                            name: str!("fizz"),
                            value_type: Some(f::n::tx(TypeExpression::Integer)),
                            default_value: None,
                        }),
                    ],
                    f::n::x(Expression::Primitive(Primitive::Nil)),
                ))],
            )));

            assert_eq!(
                super::super::analyze(&ast),
                Program(ModuleNode(
                    Module::new(
                        vec![],
                        vec![f::n::dc(
                            f::a::view(
                                "foo",
                                vec![
                                    f::n::pc(
                                        Parameter {
                                            name: str!("bar"),
                                            value_type: Some(f::n::txc(
                                                TypeExpression::Boolean,
                                                Ok(Type::Boolean)
                                            )),
                                            default_value: None,
                                        },
                                        Ok(Type::Boolean)
                                    ),
                                    f::n::pc(
                                        Parameter {
                                            name: str!("fizz"),
                                            value_type: Some(f::n::txc(
                                                TypeExpression::Integer,
                                                Ok(Type::Integer)
                                            )),
                                            default_value: None,
                                        },
                                        Ok(Type::Integer)
                                    ),
                                ],
                                f::n::xc(Expression::Primitive(Primitive::Nil), Ok(Type::Nil))
                            ),
                            Ok(Type::View(vec![1, 3]))
                        )]
                    ),
                    Ok(Type::Module(vec![(str!("foo"), RefKind::Value, 5)]))
                ))
            );
        }

        #[test]
        fn parameter_with_default_and_typedef() {
            let ast = Program(f::n::mr(Module::new(
                vec![],
                vec![f::n::d(f::a::view(
                    "foo",
                    vec![f::n::p(Parameter {
                        name: str!("bar"),
                        value_type: Some(f::n::tx(TypeExpression::Boolean)),
                        default_value: Some(f::n::x(Expression::Primitive(Primitive::Integer(
                            123,
                        )))),
                    })],
                    f::n::x(Expression::Primitive(Primitive::Nil)),
                ))],
            )));

            assert_eq!(
                super::super::analyze(&ast),
                Program(ModuleNode(
                    Module::new(
                        vec![],
                        vec![f::n::dc(
                            f::a::view(
                                "foo",
                                vec![f::n::pc(
                                    Parameter {
                                        name: str!("bar"),
                                        value_type: Some(f::n::txc(
                                            TypeExpression::Boolean,
                                            Ok(Type::Boolean)
                                        )),
                                        default_value: Some(f::n::xc(
                                            Expression::Primitive(Primitive::Integer(123)),
                                            Ok(Type::Integer)
                                        )),
                                    },
                                    Ok(Type::Boolean)
                                )],
                                f::n::xc(Expression::Primitive(Primitive::Nil), Ok(Type::Nil))
                            ),
                            Ok(Type::View(vec![2]))
                        )]
                    ),
                    Ok(Type::Module(vec![(str!("foo"), RefKind::Value, 4)]))
                ))
            );
        }

        #[test]
        fn infer_return_type_from_parameter() {
            let ast = Program(f::n::mr(Module::new(
                vec![],
                vec![f::n::d(f::a::view(
                    "foo",
                    vec![f::n::p(Parameter {
                        name: str!("bar"),
                        value_type: Some(f::n::tx(TypeExpression::Boolean)),
                        default_value: None,
                    })],
                    f::n::x(Expression::Identifier(str!("bar"))),
                ))],
            )));

            assert_eq!(
                super::super::analyze(&ast),
                Program(ModuleNode(
                    Module::new(
                        vec![],
                        vec![f::n::dc(
                            f::a::view(
                                "foo",
                                vec![f::n::pc(
                                    Parameter {
                                        name: str!("bar"),
                                        value_type: Some(f::n::txc(
                                            TypeExpression::Boolean,
                                            Ok(Type::Boolean)
                                        )),
                                        default_value: None,
                                    },
                                    Ok(Type::Boolean)
                                )],
                                f::n::xc(Expression::Identifier(str!("bar")), Ok(Type::Boolean))
                            ),
                            Ok(Type::View(vec![1]))
                        )]
                    ),
                    Ok(Type::Module(vec![(str!("foo"), RefKind::Value, 3)]))
                ))
            );
        }

        #[test]
        #[ignore = "todo"]
        fn infer_parameters() {
            let ast = Program(f::n::mr(Module::new(
                vec![],
                vec![f::n::d(f::a::view(
                    "foo",
                    vec![
                        f::n::p(Parameter {
                            name: str!("bar"),
                            value_type: None,
                            default_value: None,
                        }),
                        f::n::p(Parameter {
                            name: str!("fizz"),
                            value_type: None,
                            default_value: None,
                        }),
                    ],
                    f::n::x(Expression::BinaryOperation(
                        BinaryOperator::And,
                        Box::new(f::n::x(Expression::Identifier(str!("bar")))),
                        Box::new(f::n::x(Expression::Identifier(str!("fizz")))),
                    )),
                ))],
            )));

            assert_eq!(
                super::super::analyze(&ast),
                Program(ModuleNode(
                    Module::new(
                        vec![],
                        vec![f::n::dc(
                            f::a::view(
                                "foo",
                                vec![
                                    f::n::pc(
                                        Parameter {
                                            name: str!("bar"),
                                            value_type: None,
                                            default_value: None,
                                        },
                                        Ok(Type::Boolean)
                                    ),
                                    f::n::pc(
                                        Parameter {
                                            name: str!("fizz"),
                                            value_type: None,
                                            default_value: None,
                                        },
                                        Ok(Type::Boolean)
                                    ),
                                ],
                                f::n::xc(
                                    Expression::BinaryOperation(
                                        BinaryOperator::And,
                                        Box::new(f::n::xc(
                                            Expression::Identifier(str!("bar")),
                                            Ok(Type::Boolean)
                                        )),
                                        Box::new(f::n::xc(
                                            Expression::Identifier(str!("fizz")),
                                            Ok(Type::Boolean)
                                        )),
                                    ),
                                    Ok(Type::Boolean)
                                )
                            ),
                            Ok(Type::View(vec![0, 1]))
                        )]
                    ),
                    Ok(Type::Module(vec![(str!("foo"), RefKind::Value, 3)]))
                ))
            );
        }
    }

    mod module {
        use super::*;

        #[test]
        fn empty() {
            let ast = Program(f::n::mr(Module::new(
                vec![],
                vec![f::n::d(f::a::module(
                    "foo",
                    f::n::mr(Module::new(vec![], vec![])),
                ))],
            )));

            assert_eq!(
                super::super::analyze(&ast),
                Program(ModuleNode(
                    Module::new(
                        vec![],
                        vec![f::n::dc(
                            f::a::module(
                                "foo",
                                ModuleNode(Module::new(vec![], vec![]), Ok(Type::Module(vec![]))),
                            ),
                            Ok(Type::Module(vec![]))
                        )]
                    ),
                    Ok(Type::Module(vec![(str!("foo"), RefKind::Mixed, 1)]))
                ))
            );
        }

        #[test]
        fn with_declarations() {
            let ast = Program(f::n::mr(Module::new(
                vec![],
                vec![f::n::d(f::a::module(
                    "foo",
                    f::n::mr(Module::new(
                        vec![],
                        vec![
                            f::n::d(f::a::type_("bar", f::n::tx(TypeExpression::Boolean))),
                            f::n::d(f::a::const_(
                                "fizz",
                                None,
                                f::n::x(Expression::Primitive(Primitive::Integer(123))),
                            )),
                        ],
                    )),
                ))],
            )));

            assert_eq!(
                super::super::analyze(&ast),
                Program(ModuleNode(
                    Module::new(
                        vec![],
                        vec![f::n::dc(
                            f::a::module(
                                "foo",
                                ModuleNode(
                                    Module::new(
                                        vec![],
                                        vec![
                                            f::n::dc(
                                                f::a::type_(
                                                    "bar",
                                                    f::n::txc(
                                                        TypeExpression::Boolean,
                                                        Ok(Type::Boolean)
                                                    )
                                                ),
                                                Ok(Type::Boolean)
                                            ),
                                            f::n::dc(
                                                f::a::const_(
                                                    "fizz",
                                                    None,
                                                    f::n::xc(
                                                        Expression::Primitive(Primitive::Integer(
                                                            123
                                                        )),
                                                        Ok(Type::Integer)
                                                    ),
                                                ),
                                                Ok(Type::Integer)
                                            ),
                                        ]
                                    ),
                                    Ok(Type::Module(vec![
                                        (str!("bar"), RefKind::Type, 1),
                                        (str!("fizz"), RefKind::Value, 3),
                                    ]))
                                ),
                            ),
                            Ok(Type::Module(vec![
                                (str!("bar"), RefKind::Type, 1),
                                (str!("fizz"), RefKind::Value, 3),
                            ]))
                        )]
                    ),
                    Ok(Type::Module(vec![(str!("foo"), RefKind::Mixed, 5)]))
                ))
            );
        }
    }
}
