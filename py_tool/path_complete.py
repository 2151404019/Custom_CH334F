import os

from prompt_toolkit.completion import Completer, Completion
from prompt_toolkit import PromptSession



class PathCompleter(Completer):


    def get_completions(
            self,
            document,
            complete_event):


        text = document.text_before_cursor


        dirname = os.path.dirname(text)

        prefix = os.path.basename(text)



        if dirname == "":

            search_dir = "."

        else:

            search_dir = dirname



        try:

            files = os.listdir(search_dir)

        except:

            return



        for f in files:


            if f == "__pycache__":

                continue


            full = os.path.join(
                search_dir,
                f
            )


            if os.path.isdir(full):

                yield Completion(
                    f + "\\",
                    start_position=-len(prefix)
                )


            elif f.endswith(".txt"):

                yield Completion(
                    f,
                    start_position=-len(prefix)
                )



session = PromptSession(
    completer=PathCompleter()
)



def input_path(prompt):

    return session.prompt(prompt)