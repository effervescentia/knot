use crate::ast2::{
    self, typed,
    walk::{NodeId, Walk},
    Range,
};
use kore::invariant;
use std::collections::HashMap;

// struct Context<'a> {
//     next_id: usize,
//     types: HashMap<NodeId, typed::TypeRef<'a>>,
// }

// impl<'a> Context<'a> {
//     fn new(types: HashMap<NodeId, typed::TypeRef<'a>>) -> Self {
//         Self { next_id: 0, types }
//     }

//     fn next(&mut self) -> typed::TypeRef<'a> {
//         let id = NodeId(self.next_id);
//         self.next_id += 1;

//         self.types
//             .remove(&id)
//             .unwrap_or_else(|| invariant!("type does not exist by id {id:?}"))
//     }

//     fn wrap<Value, Result, F>(mut self, value: Value, range: Range, f: F) -> (Result, Self)
//     where
//         F: Fn(typed::Node<Value, typed::TypeRef<'a>>) -> Result,
//     {
//         (f(typed::Node::new(value, self.next(), range)), self)
//     }
// }

// #[allow(clippy::multiple_inherent_impl)]
// impl super::Program {
//     pub fn augment(self, types: HashMap<NodeId, typed::TypeRef>) -> typed::Program<typed::TypeRef> {
//         // fn bind<'a, T, R, F>(f: &'a F) -> impl Fn(T, Range, Context) -> (R, Context) + 'a
//         // where
//         //     F: Fn(typed::Node<T, typed::TypeRef>) -> R,
//         // {
//         //     move |x, r, c| c.wrap(x, r, f)
//         // }

//         let context = Context::new(types);
//         let visitor = Visitor {
//             expression: &|x, r, c: Context| {
//                 (typed::Expression(typed::Node::new(x, c.next(), r)), c)
//             },
//             statement: &|x, r, c: Context| (typed::Statement(typed::Node::new(x, c.next(), r)), c),
//             component: &|x, r, c: Context| (typed::Component(typed::Node::new(x, c.next(), r)), c),
//             type_expression: &|x, r, c: Context| {
//                 (typed::TypeExpression(typed::Node::new(x, c.next(), r)), c)
//             },
//             parameter: &|x, r, c: Context| (typed::Parameter(typed::Node::new(x, c.next(), r)), c),
//             declaration: &|x, r, c: Context| {
//                 (typed::Declaration(typed::Node::new(x, c.next(), r)), c)
//             },
//             import: &|x, r, c: Context| (typed::Import(typed::Node::new(x, c.next(), r)), c),
//             module: &|x, r, c: Context| (typed::Module(typed::Node::new(x, c.next(), r)), c),
//         };

//         let ast2::Program(module) = self.0.value;
//         let (result, _) = module.walk(&visitor, context);

//         result
//     }
// }
