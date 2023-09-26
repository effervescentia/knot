use super::weak::Weak;
use crate::{
    analyzer::{
        context::{NodeDescriptor, StrongContext, WeakContext},
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
    fn to_strong(&self, ctx: &'a StrongContext<'a>) -> R;
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
    nodes: Vec<&'a NodeDescriptor<'a>>,
    mut ctx: StrongContext<'a>,
) -> (
    Vec<&'a NodeDescriptor<'a>>,
    Vec<(&'a NodeDescriptor<'a>, String)>,
    StrongContext<'a>,
) {
    let mut unhandled = vec![];
    let mut warnings = vec![];

    nodes.into_iter().for_each(|node| {
        let mut inherit = |from_id| {
            if let Some(strong) = get_strong(&ctx.strong_refs, from_id, node.kind) {
                ctx.strong_refs
                    .insert(*node.id, ((*node.kind).clone(), strong.clone()));
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
                    .insert(**id, ((*kind).clone(), Strong::Type(x.clone())));
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
            } => match ctx.weak.bindings.resolve(scope, name, **id) {
                Some(inherit_id) => inherit(&inherit_id),

                None => {
                    ctx.strong_refs
                        .insert(**id, ((*kind).clone(), Strong::NotFound(name.clone())));
                }
            },

            _ => todo!(),
        }
    });

    (unhandled, warnings, ctx)
}

pub fn infer_types<'a>(
    nodes: &'a Vec<NodeDescriptor<'a>>,
    weak_ctx: &'a WeakContext,
) -> StrongContext<'a> {
    let mut unhandled = nodes.iter().collect::<Vec<&NodeDescriptor>>();
    let mut ctx = StrongContext::new(weak_ctx);

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
        analyzer::{context::NodeDescriptor, fragment::Fragment, infer::weak::Weak, RefKind, Type},
        ast::{
            expression::{Expression, Primitive},
            statement::Statement,
            type_expression::TypeExpression,
        },
        test::fixture as f,
    };
    use std::collections::{BTreeSet, HashMap};

    // #[test]
    // fn infer_types() {
    //     let nodes = vec![NodeDescriptor {
    //         id: &0,
    //         kind: &RefKind::Type,
    //         scope: &vec![0, 1],
    //         fragment: &Fragment::TypeExpression(TypeExpression::Nil),
    //         weak: &Weak::Type(Type::Nil),
    //     }];
    //     let file_ctx = f::f_ctx_from(vec![
    //         (
    //             0,
    //             (vec![0, 1], Fragment::TypeExpression(TypeExpression::Nil)),
    //         ),
    //         (
    //             1,
    //             (vec![0], Fragment::Declaration(f::a::type_("MyType", 0))),
    //         ),
    //     ]);
    //     let weak_ctx = f::w_ctx_from(
    //         &file_ctx,
    //         vec![
    //             (0, (RefKind::Type, Weak::Type(Type::Nil))),
    //             (1, (RefKind::Type, Weak::Inherit(0))),
    //         ],
    //         vec![(
    //             (vec![0], String::from("MyType")),
    //             (BTreeSet::from_iter(vec![1])),
    //         )],
    //     );

    //     let result = super::infer_types(&nodes, &weak_ctx);

    //     assert_eq!(
    //         result.strong_refs,
    //         HashMap::from_iter(vec![
    //             (0, (RefKind::Type, Strong::Type(Type::Nil))),
    //             (1, (RefKind::Type, Strong::Type(Type::Nil)))
    //         ])
    //     );
    // }

    // #[test]
    // fn type_inheritance() {
    //     let file_ctx = f::f_ctx_from(vec![
    //         (
    //             0,
    //             (
    //                 vec![0, 1],
    //                 Fragment::Expression(Expression::Primitive(Primitive::Nil)),
    //             ),
    //         ),
    //         (
    //             1,
    //             (vec![0], Fragment::Declaration(f::a::const_("FOO", None, 0))),
    //         ),
    //         (
    //             2,
    //             (
    //                 vec![0, 2],
    //                 Fragment::Expression(Expression::Identifier(String::from("FOO"))),
    //             ),
    //         ),
    //         (
    //             3,
    //             (vec![0], Fragment::Declaration(f::a::const_("BAR", None, 2))),
    //         ),
    //     ]);
    //     let weak_ctx = f::w_ctx_from(
    //         &file_ctx,
    //         vec![
    //             (0, (RefKind::Value, Weak::Type(Type::Nil))),
    //             (1, (RefKind::Value, Weak::Inherit(0))),
    //             (2, (RefKind::Value, Weak::Type(Type::Nil))),
    //             (3, (RefKind::Value, Weak::Inherit(2))),
    //         ],
    //         vec![
    //             (
    //                 (vec![0], String::from("FOO")),
    //                 (BTreeSet::from_iter(vec![1])),
    //             ),
    //             (
    //                 (vec![0], String::from("BAR")),
    //                 (BTreeSet::from_iter(vec![3])),
    //             ),
    //         ],
    //     );

    //     let result = super::infer_types(&vec![], &weak_ctx);

    //     assert_eq!(
    //         result.strong_refs,
    //         HashMap::from_iter(vec![
    //             (0, (RefKind::Value, Strong::Type(Type::Nil))),
    //             (1, (RefKind::Value, Strong::Type(Type::Nil))),
    //             (2, (RefKind::Value, Strong::Type(Type::Nil))),
    //             (3, (RefKind::Value, Strong::Type(Type::Nil))),
    //         ])
    //     );
    // }

    // #[test]
    // fn scope_inheritance() {
    //     let file_ctx = f::f_ctx_from(vec![
    //         (
    //             0,
    //             (
    //                 vec![0, 1],
    //                 Fragment::Expression(Expression::Primitive(Primitive::Nil)),
    //             ),
    //         ),
    //         (
    //             1,
    //             (vec![0], Fragment::Declaration(f::a::const_("FOO", None, 0))),
    //         ),
    //         (
    //             2,
    //             (
    //                 vec![0, 2, 3],
    //                 Fragment::Expression(Expression::Identifier(String::from("FOO"))),
    //             ),
    //         ),
    //         (
    //             3,
    //             (
    //                 vec![0, 2, 3],
    //                 Fragment::Statement(Statement::Variable(String::from("bar"), 2)),
    //             ),
    //         ),
    //         (
    //             4,
    //             (
    //                 vec![0, 2, 3],
    //                 Fragment::Expression(Expression::Identifier(String::from("bar"))),
    //             ),
    //         ),
    //         (
    //             5,
    //             (vec![0, 2, 3], Fragment::Statement(Statement::Expression(4))),
    //         ),
    //         (
    //             6,
    //             (
    //                 vec![0, 2],
    //                 Fragment::Expression(Expression::Closure(vec![3, 5])),
    //             ),
    //         ),
    //         (
    //             7,
    //             (
    //                 vec![0],
    //                 Fragment::Declaration(f::a::const_("FIZZ", None, 6)),
    //             ),
    //         ),
    //     ]);
    //     let weak_ctx = f::w_ctx_from(
    //         &file_ctx,
    //         vec![
    //             (0, (RefKind::Value, Weak::Type(Type::Nil))),
    //             (1, (RefKind::Value, Weak::Inherit(0))),
    //             (2, (RefKind::Value, Weak::Infer)),
    //             (3, (RefKind::Value, Weak::Inherit(2))),
    //             (4, (RefKind::Value, Weak::Infer)),
    //             (5, (RefKind::Value, Weak::Inherit(4))),
    //             (6, (RefKind::Value, Weak::Inherit(5))),
    //             (7, (RefKind::Value, Weak::Inherit(6))),
    //         ],
    //         vec![
    //             (
    //                 (vec![0], String::from("FOO")),
    //                 (BTreeSet::from_iter(vec![1])),
    //             ),
    //             (
    //                 (vec![0, 2, 3], String::from("bar")),
    //                 (BTreeSet::from_iter(vec![3])),
    //             ),
    //             (
    //                 (vec![0], String::from("FIZZ")),
    //                 (BTreeSet::from_iter(vec![7])),
    //             ),
    //         ],
    //     );

    //     let result = super::infer_types(&vec![], &weak_ctx);

    //     assert_eq!(
    //         result.strong_refs,
    //         HashMap::from_iter(vec![
    //             (0, (RefKind::Value, Strong::Type(Type::Nil))),
    //             (1, (RefKind::Value, Strong::Type(Type::Nil))),
    //             (2, (RefKind::Value, Strong::Type(Type::Nil))),
    //             (3, (RefKind::Value, Strong::Type(Type::Nil))),
    //             (4, (RefKind::Value, Strong::Type(Type::Nil))),
    //             (5, (RefKind::Value, Strong::Type(Type::Nil))),
    //             (6, (RefKind::Value, Strong::Type(Type::Nil))),
    //             (7, (RefKind::Value, Strong::Type(Type::Nil))),
    //         ])
    //     );
    // }
}
