use lang::{NodeId, ScopeId};
use std::collections::{BTreeSet, HashMap};

pub trait ResolveTarget {
    fn id(&self) -> &NodeId;
    fn scope(&self) -> &ScopeId;
}

struct NodeTarget<'a>(&'a NodeId, &'a ScopeId);

impl<'a> ResolveTarget for NodeTarget<'a> {
    fn id(&self) -> &NodeId {
        self.0
    }

    fn scope(&self) -> &ScopeId {
        self.1
    }
}

#[derive(Debug, Default, PartialEq)]
pub struct BindingMap(pub HashMap<(ScopeId, String), BTreeSet<NodeId>>);

impl BindingMap {
    pub fn resolve<Target>(&self, target: &Target, name: &str) -> Option<NodeId>
    where
        Target: ResolveTarget,
    {
        let scope = target.scope();
        let source_ids = self.0.get(&(scope.clone(), name.to_owned()));

        if let Some(xs) = source_ids {
            for x in xs.iter().rev() {
                if x < target.id() {
                    return Some(*x);
                }
            }
        }

        if scope.0.len() > 1 {
            let parent_scope = ScopeId(scope.0.get(..scope.0.len() - 1)?.to_vec());
            let parent_target = NodeTarget(target.id(), &parent_scope);

            self.resolve(&parent_target, name)
        } else {
            None
        }
    }
}

#[cfg(test)]
mod tests {
    use super::BindingMap;
    use crate::{fixture, infer::bindings::NodeTarget};
    use kore::assert_eq;
    use lang::{NodeId, ScopeId};

    #[test]
    fn import() {
        let bindings = BindingMap(fixture::import::bindings());

        assert_eq!(
            bindings.resolve(&NodeTarget(&NodeId(100), &ScopeId(vec![0])), "fizz"),
            Some(NodeId(0))
        );
    }

    #[test]
    fn type_alias() {
        let bindings = BindingMap(fixture::type_alias::bindings());

        assert_eq!(
            bindings.resolve(&NodeTarget(&NodeId(100), &ScopeId(vec![0])), "MyTypeAlias"),
            Some(NodeId(1))
        );
    }

    #[test]
    fn constant() {
        let bindings = BindingMap(fixture::constant::bindings());

        assert_eq!(
            bindings.resolve(&NodeTarget(&NodeId(100), &ScopeId(vec![0])), "MY_CONSTANT"),
            Some(NodeId(2))
        );
    }

    #[test]
    fn enumerated() {
        let bindings = BindingMap(fixture::enumerated::bindings());

        assert_eq!(
            bindings.resolve(&NodeTarget(&NodeId(100), &ScopeId(vec![0])), "MyEnum"),
            Some(NodeId(2))
        );
    }

    #[test]
    fn function() {
        let bindings = BindingMap(fixture::function::bindings());

        assert_eq!(
            bindings.resolve(&NodeTarget(&NodeId(100), &ScopeId(vec![0])), "my_function"),
            Some(NodeId(11))
        );
    }

    #[test]
    fn function_closure() {
        let bindings = BindingMap(fixture::function::bindings());

        assert_eq!(
            bindings.resolve(&NodeTarget(&NodeId(6), &ScopeId(vec![0, 1])), "first"),
            Some(NodeId(0))
        );
        assert_eq!(
            bindings.resolve(&NodeTarget(&NodeId(7), &ScopeId(vec![0, 1])), "second"),
            Some(NodeId(2))
        );
        assert_eq!(
            bindings.resolve(&NodeTarget(&NodeId(9), &ScopeId(vec![0, 1])), "third"),
            Some(NodeId(4))
        );
    }

    #[test]
    fn view() {
        let bindings = BindingMap(fixture::view::bindings());

        assert_eq!(
            bindings.resolve(&NodeTarget(&NodeId(100), &ScopeId(vec![0])), "MyView"),
            Some(NodeId(20))
        );
    }

    #[test]
    fn view_closure() {
        let bindings = BindingMap(fixture::view::bindings());

        assert_eq!(
            bindings.resolve(&NodeTarget(&NodeId(10), &ScopeId(vec![0, 1, 2])), "value"),
            Some(NodeId(6))
        );
        assert_eq!(
            bindings.resolve(&NodeTarget(&NodeId(13), &ScopeId(vec![0, 1, 2])), "inner"),
            Some(NodeId(3))
        );
    }

    #[test]
    fn module() {
        let bindings = BindingMap(fixture::module::bindings());

        assert_eq!(
            bindings.resolve(&NodeTarget(&NodeId(100), &ScopeId(vec![0])), "my_module"),
            Some(NodeId(6))
        );
    }

    #[test]
    fn module_closure() {
        let bindings = BindingMap(fixture::module::bindings());

        assert_eq!(
            bindings.resolve(&NodeTarget(&NodeId(100), &ScopeId(vec![0, 1])), "Buzz"),
            Some(NodeId(0))
        );
        assert_eq!(
            bindings.resolve(&NodeTarget(&NodeId(100), &ScopeId(vec![0, 1])), "MY_STYLE"),
            Some(NodeId(4))
        );
    }
}
