use super::data::{Action, Type};
use crate::Context;
use kore::{invariant, Serializable};
use lang::{ast, Namespace};
use std::str::FromStr;

pub fn infer<Library>(ctx: &Context<Library>, source: &ast::ImportSource, path: &[String]) -> Action
where
    Library: Serializable + FromStr,
{
    let current_path = ctx.namespace.to_path("kn");
    let namespace = Namespace::from_path(current_path, source, path);
    let module = ctx.modules.get_module_type(&namespace).unwrap_or_else(|| {
        invariant!(
            "module could not be found with path {}",
            namespace.to_path("kn").display()
        )
    });

    Action::Infer(Type::Inherit(module.0))
}

// #[cfg(test)]
// mod tests {
//     use crate::{
//         error::ResolveError,
//         infer::strong::{
//             data::{Action, Type},
//             state::State,
//         },
//         Context, ModuleMap,
//     };
//     use kore::assert_eq;
//     use lang::{
//         types::{self, Enumerated, Kind},
//         CanonicalId, NodeId,
//     };

//     #[test]
//     fn infer_function_result() {
//         let modules = ModuleMap::default();
//         let ctx = Context::mock(&modules);
//         let state = State::from_types(
//             &ctx,
//             vec![
//                 (
//                     NodeId(1),
//                     (
//                         Kind::Value,
//                         Ok(Type::Local(types::Type::Function(
//                             vec![],
//                             CanonicalId::mock(2),
//                         ))),
//                     ),
//                 ),
//                 (
//                     NodeId(2),
//                     (Kind::Value, Ok(Type::Local(types::Type::Integer))),
//                 ),
//             ],
//         );

//         assert_eq!(
//             super::infer(&state, CanonicalId::mock(1), &Kind::Value),
//             Action::Infer(Type::Inherit(CanonicalId::mock(2)))
//         );
//     }

//     #[test]
//     fn infer_enumerated_instance() {
//         let modules = ModuleMap::default();
//         let ctx = Context::mock(&modules);
//         let state = State::from_types(
//             &ctx,
//             vec![
//                 (
//                     NodeId(1),
//                     (
//                         Kind::Value,
//                         Ok(Type::Local(types::Type::Enumerated(Enumerated::Variant(
//                             vec![],
//                             CanonicalId::mock(2),
//                         )))),
//                     ),
//                 ),
//                 (
//                     NodeId(2),
//                     (
//                         Kind::Value,
//                         Ok(Type::Local(types::Type::Enumerated(
//                             Enumerated::Declaration(vec![]),
//                         ))),
//                     ),
//                 ),
//             ],
//         );

//         assert_eq!(
//             super::infer(&state, CanonicalId::mock(1), &Kind::Value),
//             Action::Infer(Type::Local(types::Type::Enumerated(Enumerated::Instance(
//                 CanonicalId::mock(2)
//             ))))
//         );
//     }

//     #[test]
//     fn skip() {
//         let modules = ModuleMap::default();
//         let ctx = Context::mock(&modules);
//         let state = State::from_types(&ctx, vec![]);

//         assert_eq!(
//             super::infer(&state, CanonicalId::mock(1), &Kind::Value),
//             Action::Skip
//         );
//     }

//     #[test]
//     fn not_inferrable() {
//         let modules = ModuleMap::default();
//         let ctx = Context::mock(&modules);
//         let state = State::from_types(
//             &ctx,
//             vec![
//                 (
//                     NodeId(1),
//                     (Kind::Value, Ok(Type::Local(types::Type::Integer))),
//                 ),
//                 (
//                     NodeId(2),
//                     (Kind::Value, Err(ResolveError::NotInferrable(vec![]))),
//                 ),
//             ],
//         );

//         assert_eq!(
//             super::infer(&state, CanonicalId::mock(1), &Kind::Value),
//             Action::Raise(ResolveError::NotInferrable(vec![]))
//         );
//         assert_eq!(
//             super::infer(&state, CanonicalId::mock(2), &Kind::Value),
//             Action::Raise(ResolveError::NotInferrable(vec![CanonicalId::mock(2)]))
//         );
//     }
// }
