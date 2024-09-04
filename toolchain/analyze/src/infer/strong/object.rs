use super::{
    data::{Action, Type},
    state::State,
};
use crate::error::Error;
use lang::{ast, types, Canonicalize, NodeId};
use std::collections::HashMap;

pub fn infer_type(
    state: &State,
    entries: &[ast::ObjectTypeExpressionEntry<String, NodeId>],
) -> Action {
    let mut entry_map = HashMap::new();

    for entry in entries {
        match entry {
            ast::ObjectTypeExpressionEntry::Required(name, x) => {
                entry_map.insert(
                    name.clone(),
                    types::ObjectTypeEntry::Required(name.clone(), state.canonicalize(*x)),
                );
            }

            ast::ObjectTypeExpressionEntry::Optional(name, x) => {
                entry_map.insert(
                    name.clone(),
                    types::ObjectTypeEntry::Optional(name.clone(), state.canonicalize(*x)),
                );
            }

            ast::ObjectTypeExpressionEntry::Spread(x) => {
                let spread_id = state.canonicalize(*x);
                match state.resolve_type(&spread_id) {
                    Some(Ok(types::Type::Object(spread))) => {
                        for entry in spread {
                            entry_map.insert(entry.name().to_owned(), entry);
                        }
                    }

                    Some(Ok(_)) => return Action::Raise(Error::NotSpreadable(spread_id)),

                    Some(Err(_)) => return Action::Raise(Error::NotInferrable(vec![spread_id])),

                    None => return Action::Skip,
                }
            }
        };
    }

    let mut sorted_entries = entry_map.into_values().collect::<Vec<_>>();
    sorted_entries.sort_by(|l, r| l.name().cmp(r.name()));

    Action::Infer(Type::Value(types::Type::Object(sorted_entries)))
}

#[cfg(test)]
mod tests {
    use crate::{
        analyze_mock,
        error::Error,
        infer::strong::{
            data::{Action, Type},
            state::State,
        },
        Context,
    };
    use kore::str;
    use lang::{
        ast,
        types::{self, Kind},
        CanonicalId, Fragment, NodeId, ScopeId,
    };
    use std::collections::BTreeMap;

    #[allow(clippy::type_complexity)]
    fn mock_state<'a>(
        ctx: &'a Context,
        fragments: &'a BTreeMap<NodeId, (ScopeId, Fragment)>,
        types: Vec<(NodeId, (Kind, Result<Type, Error>))>,
    ) -> State<'a> {
        State {
            fragments,
            types: BTreeMap::from_iter(types),
            ..State::mock(ctx)
        }
    }

    mod infer_type {
        use super::super::infer_type;
        use super::*;
        use kore::assert_eq;

        #[test]
        fn infer_object_spread() {
            let mock = analyze_mock!();
            let ctx = mock.context();
            let fragments = BTreeMap::from_iter(vec![(
                NodeId(3),
                (
                    ScopeId::default(),
                    Fragment::Expression(ast::Expression::Identifier(str!("my_object"))),
                ),
            )]);
            let state = mock_state(
                &ctx,
                &fragments,
                vec![(
                    NodeId(3),
                    (
                        Kind::Type,
                        Ok(Type::Value(types::Type::Object(vec![
                            types::ObjectTypeEntry::Optional(str!("foo"), CanonicalId::mock(4)),
                            types::ObjectTypeEntry::Required(str!("fizz"), CanonicalId::mock(5)),
                        ]))),
                    ),
                )],
            );

            assert_eq!(
                infer_type(
                    &state,
                    &[
                        ast::ObjectTypeExpressionEntry::Required(str!("foo"), NodeId(1)),
                        ast::ObjectTypeExpressionEntry::Optional(str!("bar"), NodeId(2)),
                        ast::ObjectTypeExpressionEntry::Spread(NodeId(3))
                    ]
                ),
                Action::Infer(Type::Value(types::Type::Object(vec![
                    types::ObjectTypeEntry::Optional(str!("bar"), CanonicalId::mock(2)),
                    types::ObjectTypeEntry::Required(str!("fizz"), CanonicalId::mock(5)),
                    types::ObjectTypeEntry::Optional(str!("foo"), CanonicalId::mock(4)),
                ])))
            );
        }

        #[test]
        fn infer_multiple_object_spread() {
            let mock = analyze_mock!();
            let ctx = mock.context();
            let fragments = BTreeMap::from_iter(vec![(
                NodeId(3),
                (
                    ScopeId::default(),
                    Fragment::Expression(ast::Expression::Identifier(str!("my_object"))),
                ),
            )]);
            let state = mock_state(
                &ctx,
                &fragments,
                vec![
                    (
                        NodeId(1),
                        (
                            Kind::Type,
                            Ok(Type::Value(types::Type::Object(vec![
                                types::ObjectTypeEntry::Required(
                                    str!("foo"),
                                    CanonicalId::mock(10),
                                ),
                                types::ObjectTypeEntry::Required(
                                    str!("first"),
                                    CanonicalId::mock(11),
                                ),
                            ]))),
                        ),
                    ),
                    (
                        NodeId(3),
                        (
                            Kind::Type,
                            Ok(Type::Value(types::Type::Object(vec![
                                types::ObjectTypeEntry::Required(
                                    str!("foo"),
                                    CanonicalId::mock(12),
                                ),
                                types::ObjectTypeEntry::Required(
                                    str!("second"),
                                    CanonicalId::mock(13),
                                ),
                            ]))),
                        ),
                    ),
                    (
                        NodeId(5),
                        (
                            Kind::Type,
                            Ok(Type::Value(types::Type::Object(vec![
                                types::ObjectTypeEntry::Required(
                                    str!("foo"),
                                    CanonicalId::mock(14),
                                ),
                                types::ObjectTypeEntry::Required(
                                    str!("third"),
                                    CanonicalId::mock(15),
                                ),
                            ]))),
                        ),
                    ),
                ],
            );

            assert_eq!(
                infer_type(
                    &state,
                    &[
                        ast::ObjectTypeExpressionEntry::Spread(NodeId(1)),
                        ast::ObjectTypeExpressionEntry::Required(str!("fizz"), NodeId(2)),
                        ast::ObjectTypeExpressionEntry::Spread(NodeId(3)),
                        ast::ObjectTypeExpressionEntry::Optional(str!("buzz"), NodeId(4)),
                        ast::ObjectTypeExpressionEntry::Spread(NodeId(5))
                    ]
                ),
                Action::Infer(Type::Value(types::Type::Object(vec![
                    types::ObjectTypeEntry::Optional(str!("buzz"), CanonicalId::mock(4)),
                    types::ObjectTypeEntry::Required(str!("first"), CanonicalId::mock(11)),
                    types::ObjectTypeEntry::Required(str!("fizz"), CanonicalId::mock(2)),
                    types::ObjectTypeEntry::Required(str!("foo"), CanonicalId::mock(14)),
                    types::ObjectTypeEntry::Required(str!("second"), CanonicalId::mock(13)),
                    types::ObjectTypeEntry::Required(str!("third"), CanonicalId::mock(15)),
                ])))
            );
        }

        #[test]
        fn not_spreadable() {
            let mock = analyze_mock!();
            let ctx = mock.context();
            let fragments = BTreeMap::from_iter(vec![(
                NodeId(1),
                (
                    ScopeId::default(),
                    Fragment::Expression(ast::Expression::Identifier(str!("my_object"))),
                ),
            )]);
            let state = mock_state(
                &ctx,
                &fragments,
                vec![(
                    NodeId(1),
                    (Kind::Type, Ok(Type::Value(types::Type::Integer))),
                )],
            );

            assert_eq!(
                infer_type(&state, &[ast::ObjectTypeExpressionEntry::Spread(NodeId(1))]),
                Action::Raise(Error::NotSpreadable(CanonicalId::mock(1)))
            );
        }

        #[test]
        fn not_inferrable() {
            let mock = analyze_mock!();
            let ctx = mock.context();
            let fragments = BTreeMap::from_iter(vec![(
                NodeId(1),
                (
                    ScopeId::default(),
                    Fragment::Expression(ast::Expression::Identifier(str!("my_object"))),
                ),
            )]);
            let state = mock_state(
                &ctx,
                &fragments,
                vec![(NodeId(1), (Kind::Type, Err(Error::NotInferrable(vec![]))))],
            );

            assert_eq!(
                infer_type(&state, &[ast::ObjectTypeExpressionEntry::Spread(NodeId(1))]),
                Action::Raise(Error::NotInferrable(vec![CanonicalId::mock(1)]))
            );
        }

        #[test]
        fn skip() {
            let mock = analyze_mock!();
            let ctx = mock.context();
            let fragments = BTreeMap::from_iter(vec![(
                NodeId(1),
                (
                    ScopeId::default(),
                    Fragment::Expression(ast::Expression::Identifier(str!("my_object"))),
                ),
            )]);
            let state = mock_state(&ctx, &fragments, vec![]);

            assert_eq!(
                infer_type(&state, &[ast::ObjectTypeExpressionEntry::Spread(NodeId(1))]),
                Action::Skip
            );
        }
    }
}
