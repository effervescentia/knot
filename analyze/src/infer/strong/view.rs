use super::{
    data::{Action, Type},
    state::State,
};
use kore::invariant;
use lang::{ast, types, Canonicalize, Fragment, NodeId};

pub fn infer(state: &State, parameters: &[NodeId]) -> Action {
    let parameters = parameters
        .iter()
        .map(|id| {
            state
                .fragments
                .get(id)
                .map(|(_, fragment)| match fragment {
                    Fragment::Parameter(ast::Parameter {
                        binding,
                        default_value: Some(_),
                        ..
                    }) => {
                        types::ObjectTypeEntry::Optional(binding.clone(), state.canonicalize(*id))
                    }

                    Fragment::Parameter(ast::Parameter {
                        binding,
                        default_value: None,
                        ..
                    }) => {
                        types::ObjectTypeEntry::Required(binding.clone(), state.canonicalize(*id))
                    }

                    _ => invariant!("parameter fragment does not match"),
                })
                .unwrap_or_else(|| invariant!("fragment not found for node"))
        })
        .collect::<Vec<_>>();

    Action::Infer(Type::Value(types::Type::View(parameters)))
}

// #[cfg(test)]
// mod tests {
//     use crate::{
//         analyze_mock,
//         error::Error,
//         infer::strong::{
//             data::{Action, Type},
//             state::State,
//         },
//         Context,
//     };
//     use kore::{assert_eq, str};
//     use lang::{
//         ast,
//         types::{self, Kind},
//         CanonicalId, Fragment, NodeId, ScopeId,
//     };
//     use std::collections::BTreeMap;

//     #[allow(clippy::type_complexity)]
//     fn mock_state<'a>(
//         ctx: &'a Context,
//         fragments: &'a BTreeMap<NodeId, (ScopeId, Fragment)>,
//         types: Vec<(NodeId, (Kind, Result<Type, Error>))>,
//     ) -> State<'a> {
//         State {
//             fragments,
//             types: BTreeMap::from_iter(types),
//             ..State::mock(ctx)
//         }
//     }

//     #[test]
//     fn infer_module() {
//         let mock = analyze_mock!();
//         let ctx = mock.context();
//         let fragments = BTreeMap::from_iter(vec![
//             (
//                 NodeId(1),
//                 (
//                     ScopeId(vec![]),
//                     Fragment::Declaration(ast::Declaration::type_alias(
//                         ast::Storage::public(str!("Foo")),
//                         NodeId(2),
//                     )),
//                 ),
//             ),
//             (
//                 NodeId(3),
//                 (
//                     ScopeId(vec![]),
//                     Fragment::Declaration(ast::Declaration::constant(
//                         ast::Storage::public(str!("BAR")),
//                         None,
//                         NodeId(4),
//                     )),
//                 ),
//             ),
//         ]);
//         let state = mock_state(
//             &ctx,
//             &fragments,
//             vec![
//                 (
//                     NodeId(1),
//                     (Kind::Type, Ok(Type::Value(types::Type::Boolean))),
//                 ),
//                 (
//                     NodeId(3),
//                     (Kind::Value, Ok(Type::Value(types::Type::Integer))),
//                 ),
//             ],
//         );

//         assert_eq!(
//             super::infer(&state, &[NodeId(1), NodeId(3)]),
//             Action::Infer(Type::Value(types::Type::Module(vec![
//                 (str!("Foo"), Kind::Type, CanonicalId::mock(1)),
//                 (str!("BAR"), Kind::Value, CanonicalId::mock(3))
//             ])))
//         );
//     }

//     #[test]
//     fn skip() {
//         let mock = analyze_mock!();
//         let ctx = mock.context();
//         let fragments = BTreeMap::from_iter(vec![
//             (
//                 NodeId(1),
//                 (
//                     ScopeId(vec![]),
//                     Fragment::Declaration(ast::Declaration::type_alias(
//                         ast::Storage::public(str!("Foo")),
//                         NodeId(2),
//                     )),
//                 ),
//             ),
//             (
//                 NodeId(3),
//                 (
//                     ScopeId(vec![]),
//                     Fragment::Declaration(ast::Declaration::type_alias(
//                         ast::Storage::public(str!("Bar")),
//                         NodeId(4),
//                     )),
//                 ),
//             ),
//         ]);
//         let state = mock_state(
//             &ctx,
//             &fragments,
//             vec![(
//                 NodeId(3),
//                 (Kind::Type, Ok(Type::Value(types::Type::Integer))),
//             )],
//         );

//         assert_eq!(super::infer(&state, &[NodeId(1)]), Action::Skip);
//         assert_eq!(super::infer(&state, &[NodeId(1), NodeId(3)]), Action::Skip);
//     }
// }
