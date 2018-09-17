((latex-mode . ((eval . (add-hook 'after-save-hook
                                  (lambda () (start-process "LaTeX make" nil "make"))
                                  nil t)))))
