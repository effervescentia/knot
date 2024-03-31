pub mod data;
mod declaration;
mod expression;
mod to_weak;

pub use data::{Inference, Result, Type, Weak};
use lang::FragmentMap;

pub trait ToWeak {
    fn to_weak(&self) -> Weak;
}

pub fn infer_types(fragments: &FragmentMap) -> Result {
    let mut result = Result::new(fragments);

    for (id, (scope, fragment)) in fragments {
        result.types.insert(*id, fragment.to_weak());

        if let Some((name, from_id)) = fragment.to_binding() {
            result
                .bindings
                .0
                .entry((scope.clone(), name))
                .or_default()
                .insert(from_id.unwrap_or(*id));
        }
    }

    result
}

#[cfg(test)]
mod tests {
    use crate::{
        fixture,
        infer::{weak::Result, BindingMap},
    };
    use kore::assert_eq;
    use std::collections::BTreeMap;

    #[test]
    fn import() {
        let fragments = BTreeMap::from_iter(fixture::import::fragments());

        assert_eq!(
            super::infer_types(&fragments),
            Result {
                fragments: &fragments,
                bindings: BindingMap(fixture::import::bindings()),
                types: fixture::import::weak_types(),
            }
        );
    }

    #[test]
    fn type_alias() {
        let fragments = BTreeMap::from_iter(fixture::type_alias::fragments());

        assert_eq!(
            super::infer_types(&fragments),
            Result {
                fragments: &fragments,
                bindings: BindingMap(fixture::type_alias::bindings()),
                types: fixture::type_alias::weak_types(),
            }
        );
    }

    #[test]
    fn constant() {
        let fragments = BTreeMap::from_iter(fixture::constant::fragments());

        assert_eq!(
            super::infer_types(&fragments),
            Result {
                fragments: &fragments,
                bindings: BindingMap(fixture::constant::bindings()),
                types: fixture::constant::weak_types(),
            }
        );
    }

    #[test]
    fn enumerated() {
        let fragments = BTreeMap::from_iter(fixture::enumerated::fragments());

        assert_eq!(
            super::infer_types(&fragments),
            Result {
                fragments: &fragments,
                bindings: BindingMap(fixture::enumerated::bindings()),
                types: fixture::enumerated::weak_types()
            }
        );
    }

    #[test]
    fn function() {
        let fragments = BTreeMap::from_iter(fixture::function::fragments());

        assert_eq!(
            super::infer_types(&fragments),
            Result {
                fragments: &fragments,
                bindings: BindingMap(fixture::function::bindings()),
                types: fixture::function::weak_types()
            }
        );
    }

    #[test]
    fn view() {
        let fragments = BTreeMap::from_iter(fixture::view::fragments());

        assert_eq!(
            super::infer_types(&fragments),
            Result {
                fragments: &fragments,
                bindings: BindingMap(fixture::view::bindings()),
                types: fixture::view::weak_types(),
            }
        );
    }

    #[test]
    fn module() {
        let fragments = BTreeMap::from_iter(fixture::module::fragments());

        assert_eq!(
            super::infer_types(&fragments),
            Result {
                fragments: &fragments,
                bindings: BindingMap(fixture::module::bindings()),
                types: fixture::module::weak_types(),
            }
        );
    }
}
