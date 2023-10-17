use crate::resolve::Resolver;
use knot_language::Program;
use std::{path::Path, rc::Rc};

pub struct Engine<T, R>
where
    R: Resolver,
{
    resolver: R,
    state: T,
}

impl<R> Engine<(), R>
where
    R: Resolver,
{
    pub fn new(resolver: R) -> Self {
        Self {
            resolver,
            state: (),
        }
    }

    pub fn parse(mut self, entry: &Path) -> Engine<Vec<(&Path, Rc<String>, Program<()>)>, R> {
        let input = self.resolver.resolve(entry).unwrap();
        let (ast, _) = parse::parse(&input).unwrap();

        Engine {
            resolver: self.resolver,
            state: vec![(entry, input, ast)],
        }
    }
    // pub fn parse<'a>(
    //     mut self,
    //     entry: &'a Path,
    // ) -> Engine<Vec<(&'a Path, Rc<String>, Program<'a, ()>)>, R> {
    //     let input = self.resolver.resolve(entry).unwrap();
    //     let (ast, _) = parser::parse(&input).unwrap();

    //     Engine {
    //         resolver: self.resolver,
    //         state: vec![(entry, input.clone(), ast)],
    //     }
    // }
}

// pub struct Options<'a> {
//     pub source_dir: &'a Path,
//     pub out_dir: &'a Path,
//     pub entry: &'a Path,
// }

// pub struct Engine<'a, 'b, T> {
//     options: Options<'a>,
//     resolver: Resolver<'b>,
//     state: T,
//     // modules: HashMap<String, T>,
// }

// impl<'a, 'b> Engine<'a, 'b, ()> {
//     pub fn new(options: Options<'a>) -> Engine<'a, 'a, ()> {
//         let resolver = Resolver::FileSystem(options.source_dir);

//         Engine {
//             options,
//             resolver,
//             state: (),
//         }
//     }

//     pub fn resolve(self) -> Engine<'a, 'b, Vec<(String, Program<'b, ()>)>> {
//         let source = fs::read_to_string(self.options.entry).unwrap();
//         let (ast, _) = parser::parse(&source).unwrap();

//         Engine {
//             options: self.options,
//             resolver: self.resolver,
//             state: vec![(source, ast)],
//         }
//         // self.0.modules.values_mut().for_each(|x| {
//         //     let source = fs::read_to_string(x.path).unwrap();

//         //     x.source = Some(source);
//         // });

//         // Resolved(self.0)
//     }
// }

// mod module {
//     use knot_language::parser::Program;

//     pub struct Parsed<'a> {
//         pub source: String,
//         pub ast: Program<'a, ()>,
//     }
// }

// pub struct Idle<'a, 'b>(Engine<'a, 'b>);

// impl<'a, 'b> Idle<'a, 'b> {}

// use crate::TargetFormat;
// use knot_language::{analyzer, parser};
// use resolve::Resolver;
// use std::{collections::HashMap, fs, path::Path};

// pub struct ModuleDescriptor<'a> {
//     pub path: &'a Path,
//     pub source: Option<String>,
//     pub ast: Option<parser::Program<'a, ()>>,
// }

// impl<'a> ModuleDescriptor<'a> {
//     pub fn new(path: &'a Path) -> Self {
//         Self {
//             path,
//             source: None,
//             ast: None,
//         }
//     }
// }

// pub struct Options<'a> {
//     pub source_dir: &'a Path,
//     pub out_dir: &'a Path,
//     pub entry: &'a Path,
// }

// pub struct Engine<'a, 'b> {
//     options: Options<'a>,
//     resolver: Resolver<'b>,
//     // state: T,
//     modules: HashMap<String, ModuleDescriptor<'b>>,
// }

// impl<'a, 'b> Engine<'a, 'b> {
//     pub fn new(options: Options<'a>) -> state::Idle<'a, 'a> {
//         let resolver = Resolver::FileSystem(options.source_dir);

//         let modules = HashMap::from_iter(vec![(
//             options.entry.display().to_string(),
//             ModuleDescriptor::new(options.entry),
//         )]);

//         state::Idle(Engine {
//             options,
//             resolver,
//             modules,
//         })
//     }
// }

// mod state {
//     use super::{Engine, ModuleDescriptor};
//     use knot_language::{analyzer::infer::strong::Strong, parser};
//     use std::{fs, path::Path};

//     pub struct Idle<'a, 'b>(pub Engine<'a, 'b>);

//     impl<'a, 'b> Idle<'a, 'b> {
//         pub fn resolve(mut self) -> Resolved<'a, 'b> {
//             self.0.modules.values_mut().for_each(|x| {
//                 let source = fs::read_to_string(x.path).unwrap();

//                 x.source = Some(source);
//             });

//             Resolved(self.0)
//         }
//     }

//     pub struct Resolved<'a, 'b>(pub Engine<'a, 'b>);

//     impl<'a, 'b> Resolved<'a, 'b> {
//         pub fn parse(Self(mut engine): Self) -> Parsed<'a, 'b> {
//             engine.modules.values_mut().for_each(|x| {
//                 match x {
//                     ModuleDescriptor {
//                         source: Some(source),
//                         ..
//                     } => {
//                         let (ast, _) = parser::parse(source.as_str()).unwrap();

//                         x.ast = Some(ast);
//                         ()
//                     }

//                     _ => unreachable!(),
//                 }
//                 // let source = fs::read_to_string(x.path).unwrap();

//                 // x.source = Some(source);
//             });

//             Parsed(engine)
//             // let (ast, _) = parser::parse(self.0.source.as_str()).unwrap();
//         }
//     }

//     pub struct Parsed<'a, 'b>(pub Engine<'a, 'b>);
//     pub struct Analyzed<'a, 'b>(pub Engine<'a, 'b>);
//     pub struct Built<'a, 'b>(pub Engine<'a, 'b>);
//     pub struct Watching<'a, 'b>(pub Engine<'a, 'b>);

//     // pub enum ModuleDescriptor<'a> {
//     //     Path(&'a Path),

//     //     Resolved(String),

//     //     // Parsed(String, parser::Program<'a, ()>),
//     //     Parsed(parser::Program<'a, ()>),
//     // }
// }

// // impl<'a, 'b, T> Engine<'a, 'b, T> {
// //     // fn set_state<'c, R>(self, state: R) -> Engine<'a, 'b, R>
// //     // where
// //     //     T: 'c,
// //     //     R: 'c,
// //     // {
// //     //     Engine {
// //     //         state,
// //     //         resolver: self.resolver,
// //     //         options: self.options,
// //     //     }
// //     // }

// //     fn map_state<'c, R, F>(self, f: F) -> Engine<'a, 'b, R>
// //     where
// //         T: 'c,
// //         R: 'c,
// //         F: FnOnce(T) -> R,
// //     {
// //         Engine {
// //             state: f(self.state),
// //             resolver: self.resolver,
// //             options: self.options,
// //             modules: self.modules,
// //         }
// //     }

// //     // fn map_state_ref<R, F>(self, mut f: F) -> Engine<'a, 'b, R>
// //     // where
// //     //     F: FnMut(&T) -> R,
// //     // {
// //     //     Engine {
// //     //         state: f(&self.state),
// //     //         resolver: self.resolver,
// //     //         options: self.options,
// //     //     }
// //     // }

// //     pub fn use_cache(self, cache_path: &'b Path) -> Self {
// //         Self {
// //             resolver: Resolver::Cached(cache_path, Box::new(self.resolver)),
// //             ..self
// //         }
// //     }
// // }

// // impl<'a, 'b> Engine<'a, 'b, ()> {

// //     // pub fn watch(self) -> Engine<'a, state::Watching> {}
// // }

// // impl<'a, 'b> Engine<'a, 'b, state::Resolved> {
// //     pub fn parse(
// //         Self {
// //             options,
// //             resolver,
// //             mut modules,
// //             ..
// //         }: Self,
// //     ) -> Engine<'a, 'b, state::Parsed> {
// //         modules.values_mut().for_each(|x| match x {
// //             ModuleDescriptor {
// //                 source: Some(source),
// //                 ..
// //             } => {
// //                 let (ast, _) = parser::parse(source.as_str()).unwrap();

// //                 x.ast = Some(ast);
// //             }

// //             _ => unreachable!(),
// //         });

// //         Engine {
// //             options,
// //             resolver,
// //             state: state::Parsed,
// //             modules,
// //         }
// //         // self.map_state(|_| state::Parsed)

// //         // let input =
// //         // let (ast, _) = parser::parse(self.state.0.as_str()).unwrap();

// //         // Engine {
// //         //     options: self.options,
// //         //     resolver: self.resolver,
// //         //     state: state::Parsed("", ast),
// //         // }
// //         // self.set_state(state::Parsed(&self.state.0, ast))
// //         // self.map_state_ref(|state| {
// //         //     let (ast, _) = parser::parse(state.0.as_str()).unwrap();

// //         //     state::Parsed(ast)
// //         // })
// //     }

// //     // pub fn build(&self) {}

// //     // pub fn format(&self) {}
// // }

// // // impl<'a, 'b> Engine<'a, 'b, state::Parsed<'a>> {
// // //     pub fn analyze(self) -> Engine<'a, 'b, state::Analyzed<'a>> {
// // //         self.map_state(|state @ state::Parsed(input, ast)| {
// // //             let typed = analyzer::analyze(ast);

// // //             state::Analyzed(state, typed)
// // //         })
// // //     }
// // // }

// // // impl<'a, 'b, 'c> Engine<'a, 'b, 'c, state::Analyzed<'c>> {
// // //     pub fn generate(self, target: TargetFormat) {
// // //         // self.with_state(state::Analyzed(analyzer::analyze(self.state.0)))
// // //     }
// // // }

// // // impl<'a, 'b, 'c> Engine<'a, 'b, 'c, state::Watching> {
// // //     // pub fn stop(self) -> Engine<'a, ()> {}
// // // }
