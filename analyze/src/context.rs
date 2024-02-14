use crate::weak::Weak;
use lang::{types, Fragment, FragmentMap, NodeId, ScopeId};
use std::collections::{BTreeSet, HashMap};

#[derive(Clone, Debug, PartialEq)]
pub struct ProgramContext {
    pub fragments: FragmentMap,

    pub bindings: BindingMap,
}

impl ProgramContext {
    pub fn from_fragments(fragments: FragmentMap) -> Self {
        Self::new(fragments, BindingMap::default())
    }

    pub fn new(fragments: FragmentMap, bindings: BindingMap) -> Self {
        Self {
            fragments,
            bindings,
        }
    }
}

#[derive(Clone, Debug, Default, PartialEq)]
pub struct BindingMap(pub HashMap<(ScopeId, String), BTreeSet<NodeId>>);

impl BindingMap {
    pub fn from_iter<T: IntoIterator<Item = ((ScopeId, String), BTreeSet<NodeId>)>>(
        iter: T,
    ) -> Self {
        Self(HashMap::from_iter(iter))
    }

    pub fn resolve(&self, scope: &ScopeId, name: &str, origin_id: NodeId) -> Option<NodeId> {
        let source_ids = self.0.get(&(scope.clone(), name.to_owned()));

        if let Some(xs) = source_ids {
            for x in xs.iter().rev() {
                if x < &origin_id {
                    return Some(*x);
                }
            }
        }

        if scope.0.len() > 1 {
            let parent_scope = ScopeId(scope.0.get(..scope.0.len() - 1)?.to_vec());

            self.resolve(&parent_scope, name, origin_id)
        } else {
            None
        }
    }
}

#[derive(Clone, Debug, PartialEq)]
pub struct NodeDescriptor {
    pub id: NodeId,
    pub kind: types::RefKind,
    pub scope: ScopeId,
    pub fragment: Fragment,
    pub weak: Weak,
}

// #[cfg(test)]
// mod tests {
//     use crate::context::BindingMap;
//     use kore::str;
//     use std::collections::BTreeSet;

//     #[test]
//     fn resolve_from_local_scope() {
//         let bindings = BindingMap::from_iter(vec![
//             ((vec![0], str!("foo")), BTreeSet::from_iter(vec![0])),
//             ((vec![0], str!("bar")), BTreeSet::from_iter(vec![1])),
//             ((vec![0], str!("fizz")), BTreeSet::from_iter(vec![2])),
//         ]);

//         assert_eq!(bindings.resolve(&[0], "foo", 3), Some(0));
//     }

//     #[test]
//     fn resolve_from_ancestor_scope() {
//         let bindings = BindingMap::from_iter(vec![
//             ((vec![0], str!("foo")), BTreeSet::from_iter(vec![0])),
//             ((vec![0, 1], str!("bar")), BTreeSet::from_iter(vec![1])),
//             ((vec![0, 1, 2], str!("fizz")), BTreeSet::from_iter(vec![2])),
//         ]);

//         assert_eq!(bindings.resolve(&[0, 1, 2], "foo", 3), Some(0));
//         assert_eq!(bindings.resolve(&[0, 1, 2], "bar", 3), Some(1));
//         assert_eq!(bindings.resolve(&[0, 1, 2], "fizz", 3), Some(2));
//     }

//     #[test]
//     fn resolve_from_local_over_ancestor_scope() {
//         let bindings = BindingMap::from_iter(vec![
//             ((vec![0], str!("foo")), BTreeSet::from_iter(vec![0])),
//             ((vec![0, 1], str!("foo")), BTreeSet::from_iter(vec![2])),
//             (
//                 (vec![0, 1, 2, 3], str!("foo")),
//                 BTreeSet::from_iter(vec![3]),
//             ),
//         ]);

//         assert_eq!(bindings.resolve(&[0, 1, 2], "foo", 4), Some(2));
//     }

//     #[test]
//     fn resolve_bindings_with_respect_to_ordering() {
//         let bindings = BindingMap::from_iter(vec![(
//             (vec![0], str!("foo")),
//             BTreeSet::from_iter(vec![1, 3]),
//         )]);

//         assert_eq!(bindings.resolve(&[0], "foo", 2), Some(1));
//         assert_eq!(bindings.resolve(&[0], "foo", 0), None);
//     }
// }
