((latex-mode . ((eval . (add-hook 'after-save-hook
                                  (lambda () (TeX-command-menu "LatexMk"))
                                  nil t)))))
