use super::weak::Weak;
use crate::{
    analyzer::{
        context::{BindingMap, NodeDescriptor, StrongContext},
        fragment::Fragment,
        RefKind, Type,
    },
    ast::{expression::Expression, type_expression::TypeExpression},
};
use std::collections::HashMap;

#[derive(Clone, Debug, PartialEq)]
pub enum Strong {
    NotFound(String),
    Type(Type<usize>),
}

pub type StrongRef = (RefKind, Strong);

pub trait ToStrong<'a, R> {
    fn to_strong(&self, ctx: &'a StrongContext) -> R;
}

fn get_strong<'a>(
    strong_refs: &'a HashMap<usize, StrongRef>,
    id: &'a usize,
    kind: &'a RefKind,
) -> Option<&'a Strong> {
    strong_refs.get(id).and_then(|(found_kind, strong)| {
        if found_kind == kind || found_kind == &RefKind::Mixed {
            Some(strong)
        } else {
            None
        }
    })
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

    nodes.into_iter().for_each(|node| {
        let mut inherit = |from_id| {
            if let Some(strong) = get_strong(&ctx.strong_refs, from_id, &node.kind) {
                ctx.strong_refs
                    .insert(node.id, (node.kind.clone(), strong.clone()));
            } else {
                warnings.push((node, format!("inheritance not possible from '{}'", from_id)));
            }
        };

        match node {
            NodeDescriptor {
                id,
                kind,
                weak: Weak::Type(x),
                ..
            } => {
                ctx.strong_refs
                    .insert(*id, ((*kind).clone(), Strong::Type(x.clone())));
            }

            NodeDescriptor {
                weak: Weak::Inherit(inherit_id),
                ..
            } => inherit(inherit_id),

            NodeDescriptor {
                id,
                scope,
                kind: kind @ RefKind::Type,
                fragment: Fragment::TypeExpression(TypeExpression::Identifier(name)),
                weak: Weak::Infer,
                ..
            }
            | NodeDescriptor {
                id,
                scope,
                kind: kind @ RefKind::Value,
                fragment: Fragment::Expression(Expression::Identifier(name)),
                weak: Weak::Infer,
                ..
            } => match ctx.bindings.resolve(scope, name, *id) {
                Some(inherit_id) => inherit(&inherit_id),

                None => {
                    ctx.strong_refs
                        .insert(*id, ((*kind).clone(), Strong::NotFound(name.clone())));
                }
            },

            _ => todo!(),
        }
    });

    (unhandled, warnings, ctx)
}

pub fn infer_types<'a>(nodes: &'a Vec<NodeDescriptor>, bindings: BindingMap) -> StrongContext {
    let mut unhandled = nodes.iter().collect::<Vec<&NodeDescriptor>>();
    let mut ctx = StrongContext::new(bindings);

    while !unhandled.is_empty() {
        let unhandled_length = unhandled.len();
        let result = partial_infer_types(unhandled, ctx);

        if result.0.is_empty() {
            return result.2;
        } else if result.0.len() == unhandled_length {
            panic!("analysis failed to determine all types")
        } else {
            unhandled = result.0;
            ctx = result.2;
        }
    }

    return ctx;
}

#[cfg(test)]
mod tests {
    use super::Strong;
    use crate::{
        analyzer::{
            context::{BindingMap, NodeDescriptor, StrongContext},
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

        let (.., ctx) =
            super::partial_infer_types(nodes.iter().collect(), StrongContext::new(bindings));

        assert_eq!(
            ctx.strong_refs,
            HashMap::from_iter(vec![
                (0, (RefKind::Type, Strong::Type(Type::Nil))),
                (1, (RefKind::Type, Strong::Type(Type::Nil)))
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

        let (.., ctx) =
            super::partial_infer_types(nodes.iter().collect(), StrongContext::new(bindings));

        assert_eq!(
            ctx.strong_refs,
            HashMap::from_iter(vec![
                (0, (RefKind::Value, Strong::Type(Type::Nil))),
                (1, (RefKind::Value, Strong::Type(Type::Nil))),
                (2, (RefKind::Value, Strong::Type(Type::Nil))),
                (3, (RefKind::Value, Strong::Type(Type::Nil))),
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

        let (.., ctx) =
            super::partial_infer_types(nodes.iter().collect(), StrongContext::new(bindings));

        assert_eq!(
            ctx.strong_refs,
            HashMap::from_iter(vec![
                (0, (RefKind::Value, Strong::Type(Type::Nil))),
                (1, (RefKind::Value, Strong::Type(Type::Nil))),
                (2, (RefKind::Value, Strong::Type(Type::Nil))),
                (3, (RefKind::Value, Strong::Type(Type::Nil))),
                (4, (RefKind::Value, Strong::Type(Type::Nil))),
                (5, (RefKind::Value, Strong::Type(Type::Nil))),
                (6, (RefKind::Value, Strong::Type(Type::Nil))),
                (7, (RefKind::Value, Strong::Type(Type::Nil))),
            ])
        );
    }
}
