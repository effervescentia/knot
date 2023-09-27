use super::weak::Weak;
use crate::{
    analyzer::{
        context::{BindingMap, FragmentMap, NodeDescriptor, StrongContext},
        expression::strong::{infer_binary_operation, infer_dot_access, infer_function_call},
        fragment::Fragment,
        module::infer_module,
        RefKind, Type,
    },
    ast::{expression::Expression, module::Module, type_expression::TypeExpression},
};

#[derive(Clone, Debug, PartialEq)]
pub enum ExpectedType {
    Type(Type<usize>),
    Union(Vec<Type<usize>>),
}

#[derive(Clone, Debug, PartialEq)]
pub enum SemanticError {
    NotInferrable(Vec<usize>),

    NotFound(String),

    IllegalValueAccess((Type<usize>, usize), String),
    IllegalTypeAccess((Type<usize>, usize), String),

    ShapeMismatch((Type<usize>, usize), (Type<usize>, usize)),
    UnexpectedShape((Type<usize>, usize), ExpectedType),

    VariantNotFound((Type<usize>, usize), String),

    DeclarationNotFound((Type<usize>, usize), String),

    NotIndexable((Type<usize>, usize), String),

    NotCallable(Type<usize>, usize),
    MissingArguments((Type<usize>, usize), Vec<(Type<usize>, usize)>),
    UnexpectedArguments((Type<usize>, usize), Vec<(Type<usize>, usize)>),
    InvalidArguments(Vec<((Type<usize>, usize), (Type<usize>, usize))>),
}

pub type Strong = Result<Type<usize>, SemanticError>;

pub type StrongRef = (RefKind, Strong);

pub trait ToStrong<'a, R> {
    fn to_strong(&self, ctx: &'a StrongContext) -> R;
}

fn partial_infer_types<'a>(
    nodes: Vec<&'a NodeDescriptor>,
    mut ctx: StrongContext,
) -> (
    Vec<&'a NodeDescriptor>,
    Vec<(&'a NodeDescriptor, String)>,
    StrongContext,
) {
    let mut unhandled = vec![];
    let mut warnings = vec![];

    nodes.into_iter().for_each(|node| match node {
        NodeDescriptor {
            id,
            kind,
            weak: Weak::Type(x),
            ..
        } => {
            ctx.refs.insert(*id, ((*kind).clone(), Ok(x.clone())));
        }

        NodeDescriptor {
            weak: Weak::Inherit(inherit_id),
            ..
        } => {
            if !ctx.inherit(&node, *inherit_id) {
                unhandled.push(node);
            }
        }

        NodeDescriptor {
            id,
            scope,
            kind: kind @ RefKind::Type,
            fragment: Fragment::TypeExpression(TypeExpression::Identifier(name)),
            weak: Weak::Infer,
        }
        | NodeDescriptor {
            id,
            scope,
            kind: kind @ RefKind::Value,
            fragment: Fragment::Expression(Expression::Identifier(name)),
            weak: Weak::Infer,
        } => match ctx.bindings.resolve(scope, name, *id) {
            Some(inherit_id) => {
                if !ctx.inherit(&node, inherit_id) {
                    unhandled.push(node)
                }
            }

            None => {
                ctx.refs.insert(
                    *id,
                    ((*kind).clone(), Err(SemanticError::NotFound(name.clone()))),
                );
            }
        },

        NodeDescriptor {
            id,
            kind: RefKind::Value,
            fragment: Fragment::Expression(Expression::BinaryOperation(op, lhs, rhs)),
            weak: Weak::Infer,
            ..
        } => match infer_binary_operation(op, **lhs, **rhs, &mut ctx) {
            Some(x) => {
                ctx.refs.insert(*id, (RefKind::Value, x));
            }

            None => unhandled.push(node),
        },

        NodeDescriptor {
            id,
            kind: kind @ RefKind::Type,
            fragment: Fragment::TypeExpression(TypeExpression::DotAccess(lhs, rhs)),
            weak: Weak::Infer,
            ..
        }
        | NodeDescriptor {
            id,
            kind: kind @ RefKind::Value,
            fragment: Fragment::Expression(Expression::DotAccess(lhs, rhs)),
            weak: Weak::Infer,
            ..
        } => match infer_dot_access(**lhs, rhs.clone(), &mut ctx) {
            Some(x) => {
                ctx.refs.insert(*id, (kind.clone(), x));
            }

            None => unhandled.push(node),
        },

        NodeDescriptor {
            id,
            kind: RefKind::Value,
            fragment: Fragment::Expression(Expression::FunctionCall(lhs, arguments)),
            weak: Weak::Infer,
            ..
        } => match infer_function_call(**lhs, arguments, &mut ctx) {
            Some(x) => {
                ctx.refs.insert(*id, (RefKind::Value, x));
            }

            None => unhandled.push(node),
        },

        NodeDescriptor {
            id,
            kind: RefKind::Mixed,
            fragment: Fragment::Module(Module { declarations, .. }),
            weak: Weak::Infer,
            ..
        } => match infer_module(declarations, &mut ctx) {
            Some(x) => {
                ctx.refs.insert(*id, (RefKind::Value, x));
            }

            None => unhandled.push(node),
        },

        NodeDescriptor {
            weak: Weak::Infer,
            fragment: Fragment::Parameter(_),
            ..
        } => todo!(),

        NodeDescriptor {
            weak: Weak::Infer, ..
        } => unreachable!("all other inference should be done already"),
    });

    (unhandled, warnings, ctx)
}

pub fn infer_types<'a>(
    nodes: &'a Vec<NodeDescriptor>,
    fragments: FragmentMap,
    bindings: BindingMap,
) -> StrongContext {
    let mut unhandled = nodes.iter().collect::<Vec<&NodeDescriptor>>();
    let mut ctx = StrongContext::new(fragments, bindings);

    while !unhandled.is_empty() {
        let unhandled_length = unhandled.len();
        let (next_unhandled, _, next_ctx) = partial_infer_types(unhandled, ctx);

        if next_unhandled.is_empty() {
            return next_ctx;
        } else if next_unhandled.len() == unhandled_length {
            panic!("analysis failed to determine all types")
        } else {
            unhandled = next_unhandled;
            ctx = next_ctx;
        }
    }

    return ctx;
}

#[cfg(test)]
mod tests {
    use crate::{
        analyzer::{
            context::{BindingMap, FragmentMap, NodeDescriptor, StrongContext},
            fragment::Fragment,
            infer::weak::Weak,
            RefKind, Type,
        },
        ast::{
            expression::{Expression, Primitive},
            statement::Statement,
            type_expression::TypeExpression,
        },
        test::fixture as f,
    };
    use std::collections::{BTreeSet, HashMap};

    #[test]
    fn infer_types() {
        let nodes = vec![
            NodeDescriptor {
                id: 0,
                kind: RefKind::Type,
                scope: vec![0, 1],
                fragment: Fragment::TypeExpression(TypeExpression::Nil),
                weak: Weak::Type(Type::Nil),
            },
            NodeDescriptor {
                id: 1,
                kind: RefKind::Type,
                scope: vec![0],
                fragment: Fragment::Declaration(f::a::type_("MyType", 0)),
                weak: Weak::Inherit(0),
            },
        ];
        let bindings = BindingMap::from_iter(vec![(
            (vec![0], String::from("MyType")),
            (BTreeSet::from_iter(vec![1])),
        )]);

        let (.., ctx) = super::partial_infer_types(
            nodes.iter().collect(),
            StrongContext::new(FragmentMap::new(), bindings),
        );

        assert_eq!(
            ctx.refs,
            HashMap::from_iter(vec![
                (0, (RefKind::Type, Ok(Type::Nil))),
                (1, (RefKind::Type, Ok(Type::Nil)))
            ])
        );
    }

    #[test]
    fn type_inheritance() {
        let nodes = vec![
            NodeDescriptor {
                id: 0,
                kind: RefKind::Value,
                scope: vec![0, 1],
                fragment: Fragment::Expression(Expression::Primitive(Primitive::Nil)),
                weak: Weak::Type(Type::Nil),
            },
            NodeDescriptor {
                id: 1,
                kind: RefKind::Value,
                scope: vec![0],
                fragment: Fragment::Declaration(f::a::const_("FOO", None, 0)),
                weak: Weak::Inherit(0),
            },
            NodeDescriptor {
                id: 2,
                kind: RefKind::Value,
                scope: vec![0, 2],
                fragment: Fragment::Expression(Expression::Identifier(String::from("FOO"))),
                weak: Weak::Type(Type::Nil),
            },
            NodeDescriptor {
                id: 3,
                kind: RefKind::Value,
                scope: vec![0],
                fragment: Fragment::Declaration(f::a::const_("BAR", None, 2)),
                weak: Weak::Inherit(2),
            },
        ];
        let bindings = BindingMap::from_iter(vec![
            (
                (vec![0], String::from("FOO")),
                (BTreeSet::from_iter(vec![1])),
            ),
            (
                (vec![0], String::from("BAR")),
                (BTreeSet::from_iter(vec![3])),
            ),
        ]);

        let (.., ctx) = super::partial_infer_types(
            nodes.iter().collect(),
            StrongContext::new(FragmentMap::new(), bindings),
        );

        assert_eq!(
            ctx.refs,
            HashMap::from_iter(vec![
                (0, (RefKind::Value, Ok(Type::Nil))),
                (1, (RefKind::Value, Ok(Type::Nil))),
                (2, (RefKind::Value, Ok(Type::Nil))),
                (3, (RefKind::Value, Ok(Type::Nil))),
            ])
        );
    }

    #[test]
    fn scope_inheritance() {
        let nodes = vec![
            NodeDescriptor {
                id: 0,
                kind: RefKind::Value,
                scope: vec![0, 1],
                fragment: Fragment::Expression(Expression::Primitive(Primitive::Nil)),
                weak: Weak::Type(Type::Nil),
            },
            NodeDescriptor {
                id: 1,
                kind: RefKind::Value,
                scope: vec![0],
                fragment: Fragment::Declaration(f::a::const_("FOO", None, 0)),
                weak: Weak::Inherit(0),
            },
            NodeDescriptor {
                id: 2,
                kind: RefKind::Value,
                scope: vec![0, 2, 3],
                fragment: Fragment::Expression(Expression::Identifier(String::from("FOO"))),
                weak: Weak::Infer,
            },
            NodeDescriptor {
                id: 3,
                kind: RefKind::Value,
                scope: vec![0, 2, 3],
                fragment: Fragment::Statement(Statement::Variable(String::from("bar"), 2)),
                weak: Weak::Inherit(2),
            },
            NodeDescriptor {
                id: 4,
                kind: RefKind::Value,
                scope: vec![0, 2, 3],
                fragment: Fragment::Expression(Expression::Identifier(String::from("bar"))),
                weak: Weak::Infer,
            },
            NodeDescriptor {
                id: 5,
                kind: RefKind::Value,
                scope: vec![0, 2, 3],
                fragment: Fragment::Statement(Statement::Expression(4)),
                weak: Weak::Inherit(4),
            },
            NodeDescriptor {
                id: 6,
                kind: RefKind::Value,
                scope: vec![0, 2],
                fragment: Fragment::Expression(Expression::Closure(vec![3, 5])),
                weak: Weak::Inherit(5),
            },
            NodeDescriptor {
                id: 7,
                kind: RefKind::Value,
                scope: vec![0],
                fragment: Fragment::Declaration(f::a::const_("FIZZ", None, 6)),
                weak: Weak::Inherit(6),
            },
        ];
        let bindings = BindingMap::from_iter(vec![
            (
                (vec![0], String::from("FOO")),
                (BTreeSet::from_iter(vec![1])),
            ),
            (
                (vec![0, 2, 3], String::from("bar")),
                (BTreeSet::from_iter(vec![3])),
            ),
            (
                (vec![0], String::from("FIZZ")),
                (BTreeSet::from_iter(vec![7])),
            ),
        ]);

        let (.., ctx) = super::partial_infer_types(
            nodes.iter().collect(),
            StrongContext::new(FragmentMap::new(), bindings),
        );

        assert_eq!(
            ctx.refs,
            HashMap::from_iter(vec![
                (0, (RefKind::Value, Ok(Type::Nil))),
                (1, (RefKind::Value, Ok(Type::Nil))),
                (2, (RefKind::Value, Ok(Type::Nil))),
                (3, (RefKind::Value, Ok(Type::Nil))),
                (4, (RefKind::Value, Ok(Type::Nil))),
                (5, (RefKind::Value, Ok(Type::Nil))),
                (6, (RefKind::Value, Ok(Type::Nil))),
                (7, (RefKind::Value, Ok(Type::Nil))),
            ])
        );
    }
}
