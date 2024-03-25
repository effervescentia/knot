use crate::weak::{Result, ToWeak};
use lang::FragmentMap;

pub fn infer_types<'a>(fragments: FragmentMap) -> Result<'a> {
    let mut result = Result::new(fragments);

    // iterate through the AST fragments
    result
        .module
        .fragments
        .0
        .iter()
        .for_each(|(id, (scope, fragment))| {
            // capture the weak type of each fragment
            result.refs.insert(*id, fragment.to_weak());

            // register any bindings discovered
            if let Some(name) = fragment.to_binding() {
                result
                    .module
                    .bindings
                    .0
                    .entry((scope.clone(), name))
                    .or_default()
                    .insert(*id);
            }
        });

    result
}

#[cfg(test)]
mod tests {
    use kore::str;
    use lang::{
        ast,
        types::{RefKind, Type},
        Fragment, FragmentMap, NodeId, ScopeId,
    };
    use std::collections::{BTreeSet, HashMap};

    use crate::weak;

    #[test]
    fn infer_types() {
        let fragments = FragmentMap::from_iter(vec![(
            NodeId(1),
            (
                ScopeId(vec![0]),
                Fragment::Declaration(ast::Declaration::type_alias(
                    ast::Storage::public(str!("MyType")),
                    NodeId(0),
                )),
            ),
        )]);

        let result = super::infer_types(fragments);

        assert_eq!(
            result.refs,
            HashMap::from_iter(vec![(
                NodeId(1),
                (RefKind::Type, weak::Type::Inherit(NodeId(0)))
            )])
        );

        assert_eq!(
            result.module.bindings.0,
            HashMap::from_iter(vec![(
                (ScopeId(vec![0]), str!("MyType")),
                (BTreeSet::from_iter(vec![NodeId(1)]))
            )])
        );
    }

    #[test]
    fn type_inheritance() {
        let fragments = FragmentMap::from_iter(vec![
            (
                NodeId(0),
                (
                    ScopeId(vec![0, 1]),
                    Fragment::Expression(ast::Expression::Primitive(ast::Primitive::Nil)),
                ),
            ),
            (
                NodeId(1),
                (
                    ScopeId(vec![0]),
                    Fragment::Declaration(ast::Declaration::constant(
                        ast::Storage::public(str!("FOO")),
                        None,
                        NodeId(0),
                    )),
                ),
            ),
            (
                NodeId(2),
                (
                    ScopeId(vec![0, 2]),
                    Fragment::Expression(ast::Expression::Identifier(str!("FOO"))),
                ),
            ),
            (
                NodeId(3),
                (
                    ScopeId(vec![0]),
                    Fragment::Declaration(ast::Declaration::constant(
                        ast::Storage::public(str!("BAR")),
                        None,
                        NodeId(2),
                    )),
                ),
            ),
            (
                NodeId(4),
                (
                    ScopeId(vec![0, 3, 4]),
                    Fragment::Expression(ast::Expression::Identifier(str!("BAR"))),
                ),
            ),
            (
                NodeId(5),
                (
                    ScopeId(vec![0, 3, 4]),
                    Fragment::Statement(ast::Statement::Variable(str!("fizz"), NodeId(4))),
                ),
            ),
            (
                NodeId(6),
                (
                    ScopeId(vec![0, 3, 4]),
                    Fragment::Expression(ast::Expression::Identifier(str!("fizz"))),
                ),
            ),
            (
                NodeId(7),
                (
                    ScopeId(vec![0, 3, 4]),
                    Fragment::Statement(ast::Statement::Expression(NodeId(6))),
                ),
            ),
            (
                NodeId(8),
                (
                    ScopeId(vec![0, 3]),
                    Fragment::Expression(ast::Expression::Closure(vec![NodeId(3), NodeId(7)])),
                ),
            ),
            (
                NodeId(9),
                (
                    ScopeId(vec![0]),
                    Fragment::Declaration(ast::Declaration::constant(
                        ast::Storage::public(str!("BUZZ")),
                        None,
                        NodeId(8),
                    )),
                ),
            ),
        ]);

        let result = super::infer_types(fragments);

        assert_eq!(
            result.refs,
            HashMap::from_iter(vec![
                (NodeId(0), (RefKind::Value, weak::Type::Local(Type::Nil))),
                (NodeId(1), (RefKind::Value, weak::Type::Inherit(NodeId(0)))),
                (
                    NodeId(2),
                    (
                        RefKind::Value,
                        weak::Type::Infer(weak::Inference::Resolve(str!("FOO")))
                    )
                ),
                (NodeId(3), (RefKind::Value, weak::Type::Inherit(NodeId(2)))),
                (
                    NodeId(4),
                    (
                        RefKind::Value,
                        weak::Type::Infer(weak::Inference::Resolve(str!("BAR")))
                    )
                ),
                (NodeId(5), (RefKind::Value, weak::Type::Local(Type::Nil))),
                (
                    NodeId(6),
                    (
                        RefKind::Value,
                        weak::Type::Infer(weak::Inference::Resolve(str!("fizz")))
                    )
                ),
                (NodeId(7), (RefKind::Value, weak::Type::Inherit(NodeId(6)))),
                (NodeId(8), (RefKind::Value, weak::Type::Inherit(NodeId(7)))),
                (NodeId(9), (RefKind::Value, weak::Type::Inherit(NodeId(8)))),
            ])
        );

        assert_eq!(
            result.module.bindings.0,
            HashMap::from_iter(vec![
                (
                    (ScopeId(vec![0]), str!("FOO")),
                    (BTreeSet::from_iter(vec![NodeId(1)]))
                ),
                (
                    (ScopeId(vec![0]), str!("BAR")),
                    (BTreeSet::from_iter(vec![NodeId(3)]))
                ),
                (
                    (ScopeId(vec![0, 3, 4]), str!("fizz")),
                    (BTreeSet::from_iter(vec![NodeId(5)]))
                ),
                (
                    (ScopeId(vec![0]), str!("BUZZ")),
                    (BTreeSet::from_iter(vec![NodeId(9)]))
                )
            ])
        );
    }

    #[test]
    fn duplicate_bindings() {
        let fragments = FragmentMap::from_iter(vec![
            (
                NodeId(1),
                (
                    ScopeId(vec![0]),
                    Fragment::Declaration(ast::Declaration::type_alias(
                        ast::Storage::public(str!("MyType")),
                        NodeId(0),
                    )),
                ),
            ),
            (
                NodeId(3),
                (
                    ScopeId(vec![0]),
                    Fragment::Declaration(ast::Declaration::type_alias(
                        ast::Storage::public(str!("MyType")),
                        NodeId(2),
                    )),
                ),
            ),
        ]);

        let result = super::infer_types(fragments);

        assert_eq!(
            result.refs,
            HashMap::from_iter(vec![
                (NodeId(1), (RefKind::Type, weak::Type::Inherit(NodeId(0)))),
                (NodeId(3), (RefKind::Type, weak::Type::Inherit(NodeId(2)))),
            ])
        );

        assert_eq!(
            result.module.bindings.0,
            HashMap::from_iter(vec![(
                (ScopeId(vec![0]), str!("MyType")),
                BTreeSet::from_iter(vec![NodeId(1), NodeId(3)])
            )])
        );
    }
}
