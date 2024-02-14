use crate::{ast::explode, context::WeakContext, weak::ToWeak};

pub fn infer_types(fragments: explode::FragmentMap) -> WeakContext {
    let mut ctx = WeakContext::new(fragments);

    ctx.program.fragments.0.iter().for_each(|(id, (scope, x))| {
        ctx.refs.insert(*id, x.to_weak());

        if let Ok(bindings) = x.to_binding() {
            for name in bindings {
                let entry = ctx
                    .program
                    .bindings
                    .0
                    .entry((scope.clone(), name))
                    .or_default();

                entry.insert(*id);
            }
        }
    });

    ctx
}

#[cfg(test)]
mod tests {
    use crate::weak::Weak;
    use kore::str;
    use lang::{
        ast::{
            self,
            explode::{self, FragmentMap, ScopeId},
            walk::NodeId,
        },
        types,
    };
    use std::collections::{BTreeSet, HashMap};

    #[test]
    fn infer_types() {
        let file_ctx = FragmentMap::from_iter(vec![(
            NodeId(1),
            (
                ScopeId(vec![0]),
                explode::Fragment::Declaration(ast::Declaration::type_alias(
                    ast::Storage::public(str!("MyType")),
                    NodeId(0),
                )),
            ),
        )]);

        let result = super::infer_types(file_ctx);

        assert_eq!(
            result.refs,
            HashMap::from_iter(vec![(
                NodeId(1),
                (types::RefKind::Type, Weak::Inherit(NodeId(0)))
            )])
        );

        assert_eq!(
            result.program.bindings.0,
            HashMap::from_iter(vec![(
                (ScopeId(vec![0]), str!("MyType")),
                (BTreeSet::from_iter(vec![NodeId(1)]))
            )])
        );
    }

    #[test]
    fn type_inheritance() {
        let file_ctx = FragmentMap::from_iter(vec![
            (
                NodeId(0),
                (
                    ScopeId(vec![0, 1]),
                    explode::Fragment::Expression(ast::Expression::Primitive(ast::Primitive::Nil)),
                ),
            ),
            (
                NodeId(1),
                (
                    ScopeId(vec![0]),
                    explode::Fragment::Declaration(ast::Declaration::constant(
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
                    explode::Fragment::Expression(ast::Expression::Identifier(str!("FOO"))),
                ),
            ),
            (
                NodeId(3),
                (
                    ScopeId(vec![0]),
                    explode::Fragment::Declaration(ast::Declaration::constant(
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
                    explode::Fragment::Expression(ast::Expression::Identifier(str!("BAR"))),
                ),
            ),
            (
                NodeId(5),
                (
                    ScopeId(vec![0, 3, 4]),
                    explode::Fragment::Statement(ast::Statement::Variable(str!("fizz"), NodeId(4))),
                ),
            ),
            (
                NodeId(6),
                (
                    ScopeId(vec![0, 3, 4]),
                    explode::Fragment::Expression(ast::Expression::Identifier(str!("fizz"))),
                ),
            ),
            (
                NodeId(7),
                (
                    ScopeId(vec![0, 3, 4]),
                    explode::Fragment::Statement(ast::Statement::Expression(NodeId(6))),
                ),
            ),
            (
                NodeId(8),
                (
                    ScopeId(vec![0, 3]),
                    explode::Fragment::Expression(ast::Expression::Closure(vec![
                        NodeId(3),
                        NodeId(7),
                    ])),
                ),
            ),
            (
                NodeId(9),
                (
                    ScopeId(vec![0]),
                    explode::Fragment::Declaration(ast::Declaration::constant(
                        ast::Storage::public(str!("BUZZ")),
                        None,
                        NodeId(8),
                    )),
                ),
            ),
        ]);

        let result = super::infer_types(file_ctx);

        assert_eq!(
            result.refs,
            HashMap::from_iter(vec![
                (
                    NodeId(0),
                    (types::RefKind::Value, Weak::Type(types::Type::Nil))
                ),
                (NodeId(1), (types::RefKind::Value, Weak::Inherit(NodeId(0)))),
                (NodeId(2), (types::RefKind::Value, Weak::Infer)),
                (NodeId(3), (types::RefKind::Value, Weak::Inherit(NodeId(2)))),
                (NodeId(4), (types::RefKind::Value, Weak::Infer)),
                (
                    NodeId(5),
                    (types::RefKind::Value, Weak::Type(types::Type::Nil))
                ),
                (NodeId(6), (types::RefKind::Value, Weak::Infer)),
                (NodeId(7), (types::RefKind::Value, Weak::Inherit(NodeId(6)))),
                (NodeId(8), (types::RefKind::Value, Weak::Inherit(NodeId(7)))),
                (NodeId(9), (types::RefKind::Value, Weak::Inherit(NodeId(8)))),
            ])
        );

        assert_eq!(
            result.program.bindings.0,
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
        let file_ctx = FragmentMap::from_iter(vec![
            (
                NodeId(1),
                (
                    ScopeId(vec![0]),
                    explode::Fragment::Declaration(ast::Declaration::type_alias(
                        ast::Storage::public(str!("MyType")),
                        NodeId(0),
                    )),
                ),
            ),
            (
                NodeId(3),
                (
                    ScopeId(vec![0]),
                    explode::Fragment::Declaration(ast::Declaration::type_alias(
                        ast::Storage::public(str!("MyType")),
                        NodeId(2),
                    )),
                ),
            ),
        ]);

        let result = super::infer_types(file_ctx);

        assert_eq!(
            result.refs,
            HashMap::from_iter(vec![
                (NodeId(1), (types::RefKind::Type, Weak::Inherit(NodeId(0)))),
                (NodeId(3), (types::RefKind::Type, Weak::Inherit(NodeId(2)))),
            ])
        );

        assert_eq!(
            result.program.bindings.0,
            HashMap::from_iter(vec![(
                (ScopeId(vec![0]), str!("MyType")),
                BTreeSet::from_iter(vec![NodeId(1), NodeId(3)])
            )])
        );
    }
}
