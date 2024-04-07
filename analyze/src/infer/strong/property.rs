use super::{
    data::{Action, Type},
    inherit,
    state::State,
};
use crate::error::Error;
use lang::{
    types::{self, Enumerated, Kind},
    CanonicalId,
};

fn infer_module(
    state: &State,
    declarations: &[(String, Kind, CanonicalId)],
    property: &str,
    allowed_kind: &Kind,
    module: &CanonicalId,
) -> Action {
    match declarations.iter().find(|(name, ..)| name == property) {
        Some((_, kind, id)) if allowed_kind.can_accept(kind) => inherit::inherit_any(state, *id),

        Some((.., id)) => Action::Raise(Error::UnexpectedKind(*id, *allowed_kind)),

        None => Action::Raise(Error::DeclarationNotFound(*module, property.to_owned())),
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

        None => Action::Raise(Error::VariantNotFound(*enumerated, property.to_owned())),
    }
}

pub fn infer(state: &State, lhs: CanonicalId, property: &str, allowed_kind: &Kind) -> Action {
    match state.resolve_any(&lhs) {
        Some(Ok(x)) => match x {
            types::Type::Module(declarations) => {
                infer_module(state, &declarations, property, allowed_kind, &lhs)
            }

            types::Type::Enumerated(Enumerated::Declaration(variants)) => {
                infer_enumerated(&variants, property, &lhs)
            }

            _ => Action::Raise(Error::NotIndexable(lhs, property.to_owned())),
        },

        Some(Err(_)) => Action::Raise(Error::NotInferrable(vec![lhs])),

        None => Action::Skip,
    }
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
    };
    use kore::{assert_eq, str};
    use lang::{
        types::{self, Enumerated, Kind},
        CanonicalId, NodeId,
    };

    #[test]
    fn infer_enum_variant() {
        let mock = analyze_mock!();
        let ctx = mock.context();
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
        let mock = analyze_mock!();
        let ctx = mock.context();
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
            Action::Raise(Error::VariantNotFound(CanonicalId::mock(1), str!("foo")))
        );
    }

    #[test]
    fn infer_module_entry() {
        let mock = analyze_mock!();
        let ctx = mock.context();
        let state = State::from_types(
            &ctx,
            vec![
                (
                    NodeId(1),
                    (
                        Kind::Value,
                        Ok(Type::Value(types::Type::Module(vec![(
                            str!("foo"),
                            Kind::Value,
                            CanonicalId::mock(2),
                        )]))),
                    ),
                ),
                (
                    NodeId(2),
                    (Kind::Value, Ok(Type::Value(types::Type::Integer))),
                ),
            ],
        );

        assert_eq!(
            super::infer(&state, CanonicalId::mock(1), "foo", &Kind::Value),
            Action::Infer(Type::Inherit(CanonicalId::mock(2)))
        );
    }

    #[test]
    fn infer_module_entry_not_found() {
        let mock = analyze_mock!();
        let ctx = mock.context();
        let state = State::from_types(
            &ctx,
            vec![(
                NodeId(1),
                (Kind::Value, Ok(Type::Value(types::Type::Module(vec![])))),
            )],
        );

        assert_eq!(
            super::infer(&state, CanonicalId::mock(1), "foo", &Kind::Value),
            Action::Raise(Error::DeclarationNotFound(
                CanonicalId::mock(1),
                str!("foo")
            ))
        );
    }

    #[test]
    fn infer_module_entry_unexpected_kind() {
        let mock = analyze_mock!();
        let ctx = mock.context();
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
            Action::Raise(Error::UnexpectedKind(CanonicalId::mock(2), Kind::Value))
        );
    }

    #[test]
    fn skip() {
        let mock = analyze_mock!();
        let ctx = mock.context();
        let state = State::from_types(&ctx, vec![]);

        assert_eq!(
            super::infer(&state, CanonicalId::mock(1), "foo", &Kind::Value),
            Action::Skip
        );
    }

    #[test]
    fn not_indexable() {
        let mock = analyze_mock!();
        let ctx = mock.context();
        let state = State::from_types(
            &ctx,
            vec![(
                NodeId(1),
                (Kind::Value, Ok(Type::Value(types::Type::String))),
            )],
        );

        assert_eq!(
            super::infer(&state, CanonicalId::mock(1), "foo", &Kind::Value),
            Action::Raise(Error::NotIndexable(CanonicalId::mock(1), str!("foo")))
        );
    }

    #[test]
    fn not_inferrable() {
        let mock = analyze_mock!();
        let ctx = mock.context();
        let state = State::from_types(
            &ctx,
            vec![(NodeId(1), (Kind::Value, Err(Error::NotInferrable(vec![]))))],
        );

        assert_eq!(
            super::infer(&state, CanonicalId::mock(1), "foo", &Kind::Value),
            Action::Raise(Error::NotInferrable(vec![CanonicalId::mock(1)]))
        );
    }
}
