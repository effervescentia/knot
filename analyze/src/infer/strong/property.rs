use super::{
    data::{Action, Type},
    state::State,
};
use crate::error::ResolveError;
use lang::{
    types::{self, Enumerated, Kind},
    CanonicalId,
};

fn infer_module(
    declarations: &[(String, Kind, CanonicalId)],
    property: &str,
    allowed_kind: &Kind,
    module: &CanonicalId,
) -> Action {
    match declarations.iter().find(|(name, ..)| name == property) {
        // TODO: `inherit::inherit`
        Some((_, kind, id)) if allowed_kind.can_accept(kind) => Action::Infer(Type::Inherit(*id)),

        // TODO: surface the underlying error (declaration is of the wrong kind)
        Some(_) => Action::Raise(ResolveError::NotInferrable(vec![])),

        None => Action::Raise(ResolveError::DeclarationNotFound(
            *module,
            property.to_owned(),
        )),
    }
}

fn infer_enumerated(
    variants: &[(String, Vec<CanonicalId>)],
    property: &str,
    enumerated: &CanonicalId,
) -> Action {
    match variants.iter().find(|(name, _)| name == property) {
        Some((_, parameters)) => Action::Infer(Type::Value(types::Type::Enumerated(
            Enumerated::Variant(parameters.clone(), *enumerated),
        ))),

        None => Action::Raise(ResolveError::VariantNotFound(
            *enumerated,
            property.to_owned(),
        )),
    }
}

pub fn infer(state: &State, lhs: CanonicalId, property: &str, allowed_kind: &Kind) -> Action {
    match state.resolve_any(&lhs) {
        Some(Ok(x)) => match x {
            types::Type::Module(declarations) => {
                infer_module(&declarations, property, allowed_kind, &lhs)
            }

            types::Type::Enumerated(Enumerated::Declaration(variants)) => {
                infer_enumerated(&variants, property, &lhs)
            }

            _ => Action::Raise(ResolveError::NotIndexable(lhs, property.to_owned())),
        },

        Some(Err(_)) => Action::Raise(ResolveError::NotInferrable(vec![lhs])),

        None => Action::Skip,
    }
}

#[cfg(test)]
mod tests {
    use crate::{
        error::ResolveError,
        infer::strong::{
            data::{Action, Type},
            state::State,
        },
        Context, ModuleMap,
    };
    use kore::{assert_eq, str};
    use lang::{
        types::{self, Enumerated, Kind},
        CanonicalId, NodeId,
    };

    #[test]
    fn infer_enum_variant() {
        let modules = ModuleMap::default();
        let ctx = Context::mock(&modules);
        let state = State::from_types(
            &ctx,
            vec![(
                NodeId(1),
                (
                    Kind::Value,
                    Ok(Type::Value(types::Type::Enumerated(
                        Enumerated::Declaration(vec![(
                            str!("foo"),
                            vec![CanonicalId::mock(2), CanonicalId::mock(3)],
                        )]),
                    ))),
                ),
            )],
        );

        assert_eq!(
            super::infer(&state, CanonicalId::mock(1), "foo", &Kind::Value),
            Action::Infer(Type::Value(types::Type::Enumerated(Enumerated::Variant(
                vec![CanonicalId::mock(2), CanonicalId::mock(3)],
                CanonicalId::mock(1)
            ))))
        );
    }

    #[test]
    fn infer_enum_variant_not_found() {
        let modules = ModuleMap::default();
        let ctx = Context::mock(&modules);
        let state = State::from_types(
            &ctx,
            vec![(
                NodeId(1),
                (
                    Kind::Value,
                    Ok(Type::Value(types::Type::Enumerated(
                        Enumerated::Declaration(vec![]),
                    ))),
                ),
            )],
        );

        assert_eq!(
            super::infer(&state, CanonicalId::mock(1), "foo", &Kind::Value),
            Action::Raise(ResolveError::VariantNotFound(
                CanonicalId::mock(1),
                str!("foo")
            ))
        );
    }

    #[test]
    fn infer_module_entry() {
        let modules = ModuleMap::default();
        let ctx = Context::mock(&modules);
        let state = State::from_types(
            &ctx,
            vec![(
                NodeId(1),
                (
                    Kind::Value,
                    Ok(Type::Value(types::Type::Module(vec![(
                        str!("foo"),
                        Kind::Value,
                        CanonicalId::mock(2),
                    )]))),
                ),
            )],
        );

        assert_eq!(
            super::infer(&state, CanonicalId::mock(1), "foo", &Kind::Value),
            Action::Infer(Type::Inherit(CanonicalId::mock(2)))
        );
    }

    #[test]
    fn infer_module_entry_not_found() {
        let modules = ModuleMap::default();
        let ctx = Context::mock(&modules);
        let state = State::from_types(
            &ctx,
            vec![(
                NodeId(1),
                (Kind::Value, Ok(Type::Value(types::Type::Module(vec![])))),
            )],
        );

        assert_eq!(
            super::infer(&state, CanonicalId::mock(1), "foo", &Kind::Value),
            Action::Raise(ResolveError::DeclarationNotFound(
                CanonicalId::mock(1),
                str!("foo")
            ))
        );
    }

    #[test]
    fn infer_module_entry_not_inferrable() {
        let modules = ModuleMap::default();
        let ctx = Context::mock(&modules);
        let state = State::from_types(
            &ctx,
            vec![(
                NodeId(1),
                (
                    Kind::Value,
                    Ok(Type::Value(types::Type::Module(vec![(
                        str!("foo"),
                        Kind::Type,
                        CanonicalId::mock(2),
                    )]))),
                ),
            )],
        );

        assert_eq!(
            super::infer(&state, CanonicalId::mock(1), "foo", &Kind::Value),
            Action::Raise(ResolveError::NotInferrable(vec![]))
        );
    }

    #[test]
    fn skip() {
        let modules = ModuleMap::default();
        let ctx = Context::mock(&modules);
        let state = State::from_types(&ctx, vec![]);

        assert_eq!(
            super::infer(&state, CanonicalId::mock(1), "foo", &Kind::Value),
            Action::Skip
        );
    }

    #[test]
    fn not_indexable() {
        let modules = ModuleMap::default();
        let ctx = Context::mock(&modules);
        let state = State::from_types(
            &ctx,
            vec![(
                NodeId(1),
                (Kind::Value, Ok(Type::Value(types::Type::String))),
            )],
        );

        assert_eq!(
            super::infer(&state, CanonicalId::mock(1), "foo", &Kind::Value),
            Action::Raise(ResolveError::NotIndexable(
                CanonicalId::mock(1),
                str!("foo")
            ))
        );
    }

    #[test]
    fn not_inferrable() {
        let modules = ModuleMap::default();
        let ctx = Context::mock(&modules);
        let state = State::from_types(
            &ctx,
            vec![(
                NodeId(1),
                (Kind::Value, Err(ResolveError::NotInferrable(vec![]))),
            )],
        );

        assert_eq!(
            super::infer(&state, CanonicalId::mock(1), "foo", &Kind::Value),
            Action::Raise(ResolveError::NotInferrable(vec![CanonicalId::mock(1)]))
        );
    }
}
