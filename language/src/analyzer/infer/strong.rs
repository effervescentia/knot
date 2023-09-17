use crate::{
    analyzer::{context::AnalyzeContext, fragment::Fragment, RefKind, Weak},
    parser::expression::Expression,
};

pub fn infer_types(ctx: &mut AnalyzeContext) {
    ctx.file
        .borrow()
        .fragments
        .iter()
        .for_each(|(id, (scope, fragment))| {
            let mut inherit = |from_id, to_id, to_kind| {
                if let Some((from_kind, strong)) = ctx.strong_refs.get(from_id) {
                    if *from_kind == to_kind {
                        ctx.strong_refs.insert(to_id, (to_kind, strong.clone()));
                    } else {
                        todo!("RefType mismatch: handle this with a unique StrongType")
                    }
                } else {
                    panic!("NOT FOUND: cannot inherit from")
                }
            };

            let weak = ctx.weak_refs.get(id);
            if let Some(x) = weak {
                match x {
                    (k, Weak::Type(x)) => {
                        ctx.strong_refs.insert(*id, (k.clone(), x.clone()));
                    }

                    (k, Weak::Inherit(ref_id)) => inherit(ref_id, *id, k.clone()),

                    (RefKind::Type, Weak::Unknown) => match fragment {
                        Fragment::Expression(Expression::Identifier(name)) => {
                            let bindings = &ctx.bindings;
                            let ids = bindings.get(&(scope.clone(), name.clone()));

                            if let Some(xs) = ids {
                                let mut valid_id = None;

                                for x in xs {
                                    if x < id {
                                        valid_id = Some(x);
                                    } else {
                                        break;
                                    }
                                }

                                if let Some(x) = valid_id {
                                    inherit(x, *id, RefKind::Type)
                                } else {
                                    todo!("NOT FOUND: handle this with a unique StrongType");
                                }
                            } else {
                                todo!("NOT FOUND: handle this with a unique StrongType");
                            }
                        }

                        _ => unreachable!(),
                    },

                    _ => todo!("NOT HANDLED"),
                }
            }
        });
}

#[cfg(test)]
mod tests {
    use crate::{
        analyzer::{fragment::Fragment, RefKind, Type, Weak},
        parser::{
            expression::{primitive::Primitive, Expression},
            statement::Statement,
            types::type_expression::TypeExpression,
        },
        test::fixture as f,
    };
    use std::collections::{BTreeSet, HashMap};

    #[test]
    fn infer_types() {
        let file_ctx = f::f_ctx_from(vec![
            (
                0,
                (vec![0, 1], Fragment::TypeExpression(TypeExpression::Nil)),
            ),
            (
                1,
                (vec![0], Fragment::Declaration(f::a::type_("MyType", 0))),
            ),
        ]);
        let mut analyze_ctx = f::a_ctx_from(
            &file_ctx,
            vec![
                (0, (RefKind::Type, Weak::Type(Type::Nil))),
                (1, (RefKind::Type, Weak::Inherit(0))),
            ],
            vec![(
                (vec![0], String::from("MyType")),
                (BTreeSet::from_iter(vec![1])),
            )],
        );

        super::infer_types(&mut analyze_ctx);

        assert_eq!(
            analyze_ctx.strong_refs,
            HashMap::from_iter(vec![
                (0, (RefKind::Type, Type::Nil)),
                (1, (RefKind::Type, Type::Nil))
            ])
        );
    }

    #[test]
    fn type_inheritance() {
        let file_ctx = f::f_ctx_from(vec![
            (
                0,
                (
                    vec![0, 1],
                    Fragment::Expression(Expression::Primitive(Primitive::Nil)),
                ),
            ),
            (
                1,
                (vec![0], Fragment::Declaration(f::a::const_("FOO", None, 0))),
            ),
            (
                2,
                (
                    vec![0, 2],
                    Fragment::Expression(Expression::Identifier(String::from("FOO"))),
                ),
            ),
            (
                3,
                (vec![0], Fragment::Declaration(f::a::const_("BAR", None, 2))),
            ),
        ]);
        let mut analyze_ctx = f::a_ctx_from(
            &file_ctx,
            vec![
                (0, (RefKind::Value, Weak::Type(Type::Nil))),
                (1, (RefKind::Value, Weak::Inherit(0))),
                (2, (RefKind::Value, Weak::Type(Type::Nil))),
                (3, (RefKind::Value, Weak::Inherit(2))),
            ],
            vec![
                (
                    (vec![0], String::from("FOO")),
                    (BTreeSet::from_iter(vec![1])),
                ),
                (
                    (vec![0], String::from("BAR")),
                    (BTreeSet::from_iter(vec![3])),
                ),
            ],
        );

        super::infer_types(&mut analyze_ctx);

        assert_eq!(
            analyze_ctx.strong_refs,
            HashMap::from_iter(vec![
                (0, (RefKind::Value, Type::Nil)),
                (1, (RefKind::Value, Type::Nil)),
                (2, (RefKind::Value, Type::Nil)),
                (3, (RefKind::Value, Type::Nil)),
            ])
        );
    }

    #[test]
    fn scope_inheritance() {
        let file_ctx = f::f_ctx_from(vec![
            (
                0,
                (
                    vec![0, 1],
                    Fragment::Expression(Expression::Primitive(Primitive::Nil)),
                ),
            ),
            (
                1,
                (vec![0], Fragment::Declaration(f::a::const_("FOO", None, 0))),
            ),
            (
                2,
                (
                    vec![0, 2, 3],
                    Fragment::Expression(Expression::Identifier(String::from("FOO"))),
                ),
            ),
            (
                3,
                (
                    vec![0, 2, 3],
                    Fragment::Statement(Statement::Variable(String::from("bar"), 2)),
                ),
            ),
            (
                4,
                (
                    vec![0, 2, 3],
                    Fragment::Expression(Expression::Identifier(String::from("bar"))),
                ),
            ),
            (
                5,
                (vec![0, 2, 3], Fragment::Statement(Statement::Effect(4))),
            ),
            (
                6,
                (
                    vec![0, 2],
                    Fragment::Expression(Expression::Closure(vec![3, 5])),
                ),
            ),
            (
                7,
                (
                    vec![0],
                    Fragment::Declaration(f::a::const_("FIZZ", None, 6)),
                ),
            ),
        ]);
        let mut analyze_ctx = f::a_ctx_from(
            &file_ctx,
            vec![
                (0, (RefKind::Value, Weak::Type(Type::Nil))),
                (1, (RefKind::Value, Weak::Inherit(0))),
                (2, (RefKind::Value, Weak::Unknown)),
                (3, (RefKind::Value, Weak::Inherit(2))),
                (4, (RefKind::Value, Weak::Unknown)),
                (5, (RefKind::Value, Weak::Inherit(4))),
                (6, (RefKind::Value, Weak::Inherit(5))),
                (7, (RefKind::Value, Weak::Inherit(6))),
            ],
            vec![
                (
                    (vec![0], String::from("FOO")),
                    (BTreeSet::from_iter(vec![1])),
                ),
                (
                    (vec![0, 2, 3], String::from("bar")),
                    (BTreeSet::from_iter(vec![3])),
                ),
                (
                    (vec![0], String::from("FIZZ")),
                    (BTreeSet::from_iter(vec![7])),
                ),
            ],
        );

        super::infer_types(&mut analyze_ctx);

        assert_eq!(
            analyze_ctx.strong_refs,
            HashMap::from_iter(vec![
                (0, (RefKind::Value, Type::Nil)),
                (1, (RefKind::Value, Type::Nil)),
                (2, (RefKind::Value, Type::Nil)),
                (3, (RefKind::Value, Type::Nil)),
            ])
        );
    }
}
