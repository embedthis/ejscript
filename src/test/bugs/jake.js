var jake = function = (function () {

 var tasks = {};

 var invoke = function (task) {

   if (task.done) {
     //
   } else {
     //invoke depends
     task.depends.forEach(function (el, i, ary) {
       invoke(tasks[el]);
     });

     task.work();
     task.done = true;
     print(task.desc);
   }

 };

 return {
   task: function (name, desc, depends, fn) {
     tasks[name] = {
       desc: desc,
       depends: depends,
       work: fn,
       done: false
     }
   },
   run: function (name) {
     invoke(tasks[name]);
   }
 };
})();

var task = jake.task;
var run = jake.run;
